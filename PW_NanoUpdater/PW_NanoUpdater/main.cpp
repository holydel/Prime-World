#include <windows.h>
#include <CommCtrl.h>
#include <iostream>
#include <git2.h>

#include <locale>
#include <codecvt>
#include <string>
#include <vector>
#include <stdexcept>

#include <format>
#include <filesystem>
#include <io.h>
#include <thread>
#include <fstream>
#include <functional>

class admin_rights_exception : std::exception {

};

static int error;

static bool isAdminRightsRequired = false;

static bool isRunningAdm = false;

static LPSTR progressBarHandle = nullptr;

int SocketListen(std::atomic<bool>& doWork);
int SocketTrancieve(const char* argv, std::atomic<bool>& doWork);
void TransmitMessage(char const* strbuf, std::streamsize strSize);
void DownloadRelease(const std::string& fileUrl, const std::string& filePath, const std::string& md5Path);

bool gh_fs_rm(const std::string& path) {
   WIN32_FIND_DATA findFileData;
   HANDLE hFind = INVALID_HANDLE_VALUE;

   // Добавляем к пути "\\*" для поиска всех файлов
   std::string strPath = path + "\\*";

   hFind = FindFirstFile(strPath.c_str(), &findFileData);
   if (hFind == INVALID_HANDLE_VALUE) {
      return false;
   }

   do {
      const std::string fileName = findFileData.cFileName;
      const std::string fullPath = path + "\\" + fileName;

      if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
         // Не удаляем текущую (".") и родительскую ("..")иректории
         if (fileName != "." && fileName != "..") {
            // Рекурсивно удаляем вложенные директории
            if (!gh_fs_rm(fullPath)) {
               FindClose(hFind);
               return false;
            }
         }
      }
      else {
         if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY ||
            findFileData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN ||
            findFileData.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) {

            // Снимаем атрибуты, которые могут мешать удалению файла
            SetFileAttributes(fullPath.c_str(), FILE_ATTRIBUTE_NORMAL);
         }

         // Удаляем файл
         if (!DeleteFile(fullPath.c_str())) {
            DWORD err = GetLastError();
            std::cerr << "Error in DeleteFile: " << GetLastError() << std::endl << fullPath.c_str() << std::endl;
            FindClose(hFind);
            continue;
         }
      }
   } while (FindNextFile(hFind, &findFileData) != 0);

   FindClose(hFind);

   // Наконец, удаляем корневую директорию
   if (!RemoveDirectory(path.c_str())) {
      std::cerr << "Error in RemoveDirectory: " << GetLastError() << std::endl;
      return false;
   }

   return true;
}


void CheckError(const char* stage)
{
   if (error < 0) {
      if (isAdminRightsRequired) {
         std::string errorLast = git_error_last()->message;
         throw admin_rights_exception();
      } else {
         std::cerr << stage << " failed: " << git_error_last()->message << std::endl;
         throw std::exception();
      }
   }
}


int fetchhead_callback(const char* ref_name, const char* remote_url, const git_oid* oid, unsigned int is_merge, void* payload) {
   memcpy(payload, oid, sizeof(git_oid));
   return 0; // return-zero to stop iterating
}


int create_merge_commit(git_repository* repo, git_oid* merge_commit_oid, git_annotated_commit* fetched_commit) {
   git_signature* sig;
   error = git_signature_now(&sig, "Your Name", "your_email@example.com"); // Здесь используйте свои имя и email
   CheckError("Create signature");

   git_index* index;
   error = git_repository_index(&index, repo);
   CheckError("Get index");

   git_oid tree_oid;
   error = git_index_write_tree(&tree_oid, index);
   CheckError("Write tree from index");

   git_tree* tree;
   error = git_tree_lookup(&tree, repo, &tree_oid);
   CheckError("Lookup tree");

   error = git_commit_create_v(
      merge_commit_oid, repo, "HEAD", sig, sig, NULL,
      "Merge remote branch 'origin/main'", tree,
      1, git_annotated_commit_id(fetched_commit)
   );
   CheckError("Create commit");

   // Очистка
   git_signature_free(sig);
   git_index_free(index);
   git_tree_free(tree);

   return error; // Возвращаем код ошибки операции
}


int fetch_progress(const git_transfer_progress* stats, void* payload) {
   int progress = (int)max(0.f, min(100.f, (float)stats->received_objects / (float)stats->total_objects * 100.f));

   if (isRunningAdm) {
      std::string pr = std::format("{}\n", progress);
      TransmitMessage(pr.c_str(), pr.size());
   }
   else {
      if (progressBarHandle) {
         HWND hwndProgressBar = (HWND)atoi(progressBarHandle);
         int result = PostMessage(hwndProgressBar, PBM_SETRANGE, 0, 100);
         result = PostMessage(hwndProgressBar, PBM_SETPOS, progress, 0);
         auto errrr = GetLastError();
         int a = 101;
      }
      // "name": "shader_pos"
      std::cout << "#{\"type\":\"bar\", \"data\":\"" << progress << "\"}" << std::endl;
   }

   return 0;
}


void CleanAndClone(const char* repoPath, git_repository* repo, const char* remoteRepoUrl)
{
   if (isAdminRightsRequired) {
      throw admin_rights_exception();
   }
   // If opening the repository failed, delete the target directory

   // Assuming gh_fs_rm is a function that recursively deletes a directory.
   error = gh_fs_rm(repoPath);
   CheckError("Directory deletion");

   // Clone
   git_clone_options cloneOpts = GIT_CLONE_OPTIONS_INIT;
   //cloneOpts.fetch_opts.depth = 1;
   cloneOpts.fetch_opts.callbacks.transfer_progress = fetch_progress;

   error = git_clone(&repo, remoteRepoUrl, repoPath, &cloneOpts);
   CheckError("Clone");
}


void RemoteFetch(git_repository* repo, const char* remoteRepoUrl, const char* branchName)
{
   git_reference* head = nullptr;
   error = git_repository_head(&head, repo);
   CheckError("Get HEAD");
   const git_oid* local_commit = git_reference_target(head);

   git_remote* remote = nullptr;
   error = git_remote_lookup(&remote, repo, "origin");
   CheckError("Remote lookup");

   git_remote_callbacks callbacks = GIT_REMOTE_CALLBACKS_INIT;
   error = git_remote_connect(remote, GIT_DIRECTION_FETCH, &callbacks, NULL, NULL);
   CheckError("Connect");

   const git_remote_head** refs;
   size_t size;
   error = git_remote_ls(&refs, &size, remote);
   CheckError("Remote ls");

   int cmmm = git_oid_cmp(local_commit, &refs[0]->oid);

   char oid[GIT_OID_SHA1_HEXSIZE + 1] = { 0 };
   git_oid_fmt(oid, &refs[1]->oid);

   char oidLocal[GIT_OID_SHA1_HEXSIZE + 1] = { 0 };
   git_oid_fmt(oidLocal, local_commit);

   if (git_oid_cmp(local_commit, &refs[0]->oid) == 0) {
      return;
   }

   if (isAdminRightsRequired) {
      throw admin_rights_exception();
   }

   git_fetch_options fetch_opts = GIT_FETCH_OPTIONS_INIT;
   fetch_opts.callbacks.transfer_progress = fetch_progress;
   error = git_remote_fetch(remote, nullptr, &fetch_opts, nullptr);
   CheckError("Remote fetch");

   const git_oid* remote_commit = git_reference_target(head);


   git_remote_free(remote);


   // Verify if we actually fetched any new data
   git_oid fetch_head_oid;
   error = git_repository_fetchhead_foreach(repo, fetchhead_callback, &fetch_head_oid);
   CheckError("Fetching FETCH_HEAD");

   git_annotated_commit* fetched_commit = NULL;
   error = git_annotated_commit_from_fetchhead(&fetched_commit, repo, branchName, remoteRepoUrl, &fetch_head_oid);
   CheckError("Get annotated commit from FETCH_HEAD");

   git_merge_options merge_opts = GIT_MERGE_OPTIONS_INIT;
   git_checkout_options checkout_opts = GIT_CHECKOUT_OPTIONS_INIT;

   const git_annotated_commit* cmt[] = { fetched_commit };
   error = git_merge(repo, cmt, 1, &merge_opts, &checkout_opts);
   git_annotated_commit_free(fetched_commit);
   CheckError("Merge");

   if (git_repository_state(repo) == GIT_REPOSITORY_STATE_NONE) {
      git_oid merge_commit_oid;
      error = create_merge_commit(repo, &merge_commit_oid, fetched_commit);
      CheckError("Create merge commit");
   }
}


void UpdateRepo(const char* repoPath, const char* remoteRepoUrl, const char* branchName, int recursion = 0) {
   git_repository* repo = nullptr;

   // Try to open the repository
   error = git_repository_open(&repo, repoPath);
   if (error) {
      CleanAndClone(repoPath, repo, remoteRepoUrl);
   }

   // Once the repository is cloned or opened, perform the rest of the
   if (repo) {
      try {
         bool anyChanges = false;
         git_status_options statusopt = GIT_STATUS_OPTIONS_INIT;
         statusopt.show = GIT_STATUS_SHOW_INDEX_AND_WORKDIR;
         statusopt.flags = GIT_STATUS_OPT_INCLUDE_UNTRACKED | GIT_STATUS_OPT_RECURSE_UNTRACKED_DIRS;

         git_status_list* status;
         error = git_status_list_new(&status, repo, &statusopt);
         CheckError("Status List");

         size_t status_count = git_status_list_entrycount(status);
         for (size_t i = 0; i < status_count; ++i) {
            const git_status_entry* s = git_status_byindex(status, i);
            if (s->status == GIT_STATUS_WT_NEW || s->status == GIT_STATUS_WT_MODIFIED ||
               s->status == GIT_STATUS_WT_DELETED || s->status == GIT_STATUS_WT_TYPECHANGE ||
               s->status == GIT_STATUS_WT_RENAMED || s->status == GIT_STATUS_WT_UNREADABLE) {
               if (isAdminRightsRequired) {
                  throw admin_rights_exception();
               } else {
                  anyChanges = true;
                  break;
               }
            }
         }

         if (anyChanges) {
            // Reset
            git_object* head_obj = nullptr;
            error = git_revparse_single(&head_obj, repo, "HEAD");
            CheckError("Get Head");

            error = git_reset(repo, head_obj, GIT_RESET_HARD, nullptr);
            git_object_free(head_obj);
            CheckError("Test reset");

            // Identify unversioned files
            git_status_options statusopt = GIT_STATUS_OPTIONS_INIT;
            statusopt.show = GIT_STATUS_SHOW_INDEX_AND_WORKDIR;
            statusopt.flags = GIT_STATUS_OPT_INCLUDE_UNTRACKED | GIT_STATUS_OPT_RECURSE_UNTRACKED_DIRS;

            git_status_list* status;
            error = git_status_list_new(&status, repo, &statusopt);
            CheckError("Status List");
            char long_path[4096];
            size_t repoPathLength = strlen(repoPath);
            memcpy(long_path, repoPath, repoPathLength + 1);
            long_path[repoPathLength] = '\\';

            size_t status_count = git_status_list_entrycount(status);
            for (size_t i = 0; i < status_count; ++i) {
               const git_status_entry* s = git_status_byindex(status, i);
               if (s->status == GIT_STATUS_WT_NEW) {
                  // This is an untracked file
                  const char* path = s->index_to_workdir->old_file.path;

                  memcpy(long_path + repoPathLength + 1, path, strlen(path) + 1);
                  // Removing the file
                  if (remove(long_path) == -1) {
                     std::cerr << "Failed to remove untracked file:" << path << std::endl;
                  }
               }
            }
            git_status_list_free(status);
         }

         RemoteFetch(repo, remoteRepoUrl, branchName);
      }
      catch (admin_rights_exception ex) {
         throw ex;
      }
      catch (std::exception ex) {
         if (recursion == 0) {
            if (repo) {
               git_repository_free(repo);
            }
            CleanAndClone(repoPath, repo, remoteRepoUrl);
            UpdateRepo(repoPath, remoteRepoUrl, branchName, 2);
         }
         if (recursion == 1) {
            std::cerr << "Failed to fix repository. Reinstall the game\n";
            throw std::exception();
         }
      }
      git_repository_free(repo);
   }
}


int LaunchAdminNanoUpdater(std::string& cmdLine) {
   std::filesystem::path cwd = std::filesystem::current_path();

   SHELLEXECUTEINFO ShExecInfo = { 0 };
   ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
   ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
   ShExecInfo.hwnd = NULL;
   ShExecInfo.lpVerb = "open";
   ShExecInfo.lpFile = "..\\Tools\\PW_NanoUpdaterAdm.exe";
   ShExecInfo.lpParameters = cmdLine.c_str();
   ShExecInfo.lpDirectory = cwd.string().c_str();
   ShExecInfo.nShow = SW_SHOW;
   ShExecInfo.hInstApp = NULL;

   if (ShellExecuteEx(&ShExecInfo)) {

      std::atomic<bool> doWork;
      doWork.store(true);

      std::thread readStdErr([&doWork]() {
         SocketListen(doWork);
         });

      HANDLE hProcess = ShExecInfo.hProcess;
      DWORD exitCode = STILL_ACTIVE;
      while (exitCode == STILL_ACTIVE) {
         GetExitCodeProcess(hProcess, &exitCode);
         Sleep(1000);
      }
      
      doWork.store(false);
      readStdErr.join();
      
      CloseHandle(ShExecInfo.hProcess);
   }
   else {
      std::cerr << "Failed to execute command." << std::endl;
      return 1;
   }

   return 0;
}


bool IsAdminRequired() {

   std::ofstream srrr("..\\Tools\\test");
   if (!srrr.fail()) {
      srrr.close();
      remove("..\\Tools\\test");
   }
   return srrr.fail();
}


int RunWithAdminRights(const char* repoPath, const char* remoteRepoUrl, const char* branchName)
{
   static bool isAlreadyInvokedAdmin = false;
   if (isAdminRightsRequired && !isAlreadyInvokedAdmin) {
      isAlreadyInvokedAdmin = true;
      std::string cmdLine = std::format("{} {} {}", repoPath, remoteRepoUrl, branchName);
      return LaunchAdminNanoUpdater(cmdLine);
   }
   return 1;
}


int Update(const char* repoPath, const char* remoteRepoUrl, const char* branchName) {

   // Initialize the library
   int _error = git_libgit2_init();
   if (_error < 0) {
      std::cerr << "Failed to init libgit2 library!";
      return 1;
   }

#ifndef ADMIN_MANIFEST
   _error = git_libgit2_opts(GIT_OPT_SET_OWNER_VALIDATION, false);
   if (_error < 0) {
      std::cerr << "Failed to set non-admin opts for libgit2!";
      return 1;
   }
#endif

#ifndef ADMIN_MANIFEST
   isAdminRightsRequired = IsAdminRequired();
#endif

   try {
      UpdateRepo(repoPath, remoteRepoUrl, branchName);
   }
   catch (admin_rights_exception) {
#ifndef ADMIN_MANIFEST
      //std::cout << "Need admin rights" << std::endl;
      return RunWithAdminRights(repoPath, remoteRepoUrl, branchName);
#endif
      return 1;
   }
   catch (std::exception ex) {
      std::cerr << "Exception: " << ex.what();
      _error = git_libgit2_shutdown();
      return 1;
   }

   _error = git_libgit2_shutdown();
   if (_error < 0) {
      std::cerr << "Failed to shutdown libgit2 library!";
      return 1;
   }
   return 0;
}

#if 0
int main(int argc, char* argv[]) {
#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
   PSTR lpCmdLine, int nCmdShow) {
   char* winCmd = GetCommandLine();
   std::vector<char*> argVector;
   int index = 0;
   bool newOption = true;

   // walk over the command line and convert it to argv
   while (winCmd[index] != 0) {
      if (winCmd[index] == L' ') {
         // terminate option string
         winCmd[index] = 0;
         newOption = true;

      }
      else {
         if (newOption) {
            argVector.push_back(&winCmd[index]);
}
         newOption = false;
      }
      index++;
   }
   int argc = argVector.size();
   char** argv = argVector.data();
#endif
   bool skipRelease = false;
   for (int a = 0; a < argc; ++a) {
      if (std::string(argv[a]) == "inno") {
         skipRelease = true;
         if (a + 1 < argc) {
            progressBarHandle = argv[a + 1];
            HWND hwndProgressBar = (HWND)atoi(progressBarHandle);
            int result = PostMessage(hwndProgressBar, PBM_SETRANGE, 0, 100);
         }
         break;
      }
   }

#ifdef ADMIN_MANIFEST
   isRunningAdm = true;
   std::atomic<bool> doWork;
   doWork.store(true);

   std::thread writeStdOut([&doWork]() {
      SocketTrancieve("127.0.0.1", doWork);
      });

   const char szUniqueNamedMutex[] = "pwclassic_nano_updater_adm";
#else
   const char szUniqueNamedMutex[] = "pwclassic_nano_updater";
#endif
   HANDLE hHandle = CreateMutex(NULL, TRUE, szUniqueNamedMutex);
   if (ERROR_ALREADY_EXISTS == GetLastError())
   {
      CloseHandle(hHandle);
      return(1); // Exit program
   }

   const char* repoPath[] = {
      "..\\Game",
      "..\\Launcher\\content"
   };

   const char* repoLabels[] = {
      "game",
      "content"
   };

   const char* remoteRepoUrl[] = { 
      "https://github.com/Prime-World-Classic/PWCGitUpdates.git",
      "https://github.com/Prime-World-Classic/content.git"
   };
   const char* branchName = "main";

   int result = 0;
   for (int i = 0; i < _countof(repoPath); ++i) {
      std::cout << "#{\"type\":\"label\", \"data\":\"" << repoLabels[i] << "\"}" << std::endl;
      result += Update(repoPath[i], remoteRepoUrl[i], branchName);
   }

   const char* releaseFileUrls[] = {
         "https://github.com/Prime-World-Classic/Prime-World/releases/download/2.0/data01.pile",
         "https://github.com/Prime-World-Classic/Prime-World/releases/download/2.0/data02.pile",
         "https://github.com/Prime-World-Classic/Prime-World/releases/download/2.0/data03.pile",
         "https://github.com/Prime-World-Classic/Prime-World/releases/download/2.0/data04.pile",
         "https://github.com/Prime-World-Classic/Prime-World/releases/download/2.0/data05.pile",
         "https://github.com/Prime-World-Classic/Prime-World/releases/download/2.0/data06.pile",
         "https://github.com/Prime-World-Classic/Prime-World/releases/download/2.0/Asks_RU.fsb",
         "https://github.com/Prime-World-Classic/Prime-World/releases/download/2.0/Music.fsb",
   };

   const char* releaseFilePaths[] = {
      "..\\Game\\Packs\\data01.pile",
      "..\\Game\\Packs\\data02.pile",
      "..\\Game\\Packs\\data03.pile",
      "..\\Game\\Packs\\data04.pile",
      "..\\Game\\Packs\\data05.pile",
      "..\\Game\\Packs\\data06.pile",
      "..\\Game\\Data\\Audio\\Asks_RU.fsb",
      "..\\Game\\Data\\Audio\\Music.fsb"
   };

   const char* releaseFileHashes[] = {
      "..\\Game\\Hashes\\data01.pile.md5",
      "..\\Game\\Hashes\\data02.pile.md5",
      "..\\Game\\Hashes\\data03.pile.md5",
      "..\\Game\\Hashes\\data04.pile.md5",
      "..\\Game\\Hashes\\data05.pile.md5",
      "..\\Game\\Hashes\\data06.pile.md5",
      "..\\Game\\Hashes\\Asks_RU.fsb.md5",
      "..\\Game\\Hashes\\Music.fsb.md5"
   };

   if (!skipRelease) {
      std::cout << "#{\"type\":\"label\", \"data\":\"" << "game_data" << "\"}" << std::endl;
      for (int r = 0; r < _countof(releaseFileUrls); ++r) {
         DownloadRelease(releaseFileUrls[r], releaseFilePaths[r], releaseFileHashes[r]);
      }
   }

   int iad = 0;

#ifdef ADMIN_MANIFEST
   doWork.store(false);
   writeStdOut.join();
#endif
   return result;
}
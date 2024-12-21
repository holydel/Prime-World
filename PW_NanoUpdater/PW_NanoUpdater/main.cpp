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
#pragma optimize("", off)
class admin_rights_exception : std::exception {

};
struct null_streambuf : public std::streambuf
{
   using int_type = std::streambuf::int_type;
   using traits = std::streambuf::traits_type;

   virtual int_type overflow(int_type value) override
   {
      return value;
   }
};

static int error;

static bool isAdminRightsRequired = false;

bool isRunningAdm = false;

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


void ThrowIfNotWithAdminRights() {
#ifndef ADMIN_MANIFEST
   throw admin_rights_exception();
#endif
}


void CheckError(const char* stage)
{
   if (error < 0) {
      std::string errorLast = git_error_last()->message;
      std::cerr << stage << " failed: " << errorLast << std::endl;
      if (isAdminRightsRequired) {
         throw admin_rights_exception(); // Try with admin rights just in case
      } else {
         throw std::exception();
      }
   }
}


static std::string refName;
int fetchhead_callback(const char* ref_name, const char* remote_url, const git_oid* oid, unsigned int is_merge, void* payload) {
   memcpy(payload, oid, sizeof(git_oid));

   refName = ref_name;

   return 0; // return-zero to stop iterating
}


int fetch_progress(const git_transfer_progress* stats, void* payload) {
   if (stats->total_objects == 0) {
      return 0;
   }
   int progress = (int)max(0.f, min(100.f, (float)stats->received_objects / (float)stats->total_objects * 100.f));

   if (isRunningAdm) {
      std::stringstream strStream;
      strStream << "#{\"type\":\"bar\", \"data\":\"" << progress << "\"}" << std::endl;
      TransmitMessage(strStream.str().c_str(), strStream.str().size());
   }
   else {
      if (progressBarHandle) {
         HWND hwndProgressBar = (HWND)atoi(progressBarHandle);
         int result = PostMessage(hwndProgressBar, PBM_SETRANGE, 0, 100);
         result = PostMessage(hwndProgressBar, PBM_SETPOS, progress, 0);
      }
      // "name": "shader_pos"
      std::cout << "#{\"type\":\"bar\", \"data\":\"" << progress << "\"}" << std::endl;
   }

   return 0;
}


void InitRepo(git_repository** repo, git_remote** remote, const char* repoPath, const char* remoteRepoUrl)
{
   ThrowIfNotWithAdminRights();

   error = git_repository_init(repo, repoPath, false);
   CheckError("Init");

   error = git_remote_lookup(remote, *repo, "origin");
   if (error < 0) {
      error = git_remote_create(remote, *repo, "origin", remoteRepoUrl);
      CheckError("Add remote");
   }
}


void RemoteFetch(git_repository* repo, git_remote* remote, const char* remoteRepoUrl, const char* branchName)
{
   // Check if any refs exist
   git_oid fetch_head_oidTest;
   error = git_repository_fetchhead_foreach(repo, fetchhead_callback, &fetch_head_oidTest);

   // Test if fetch is needed (only if not 'clone')
   if (!refName.empty()) {
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

      error = git_remote_disconnect(remote);

      int cmmm = git_oid_cmp(local_commit, &refs[0]->oid);

      char oid[GIT_OID_SHA1_HEXSIZE + 1] = { 0 };
      git_oid_fmt(oid, &refs[1]->oid);

      char oidLocal[GIT_OID_SHA1_HEXSIZE + 1] = { 0 };
      git_oid_fmt(oidLocal, local_commit);

      if (git_oid_cmp(local_commit, &refs[0]->oid) == 0) {
         return;
      }
      ThrowIfNotWithAdminRights();
   }

   // Search for remotes
   error = git_remote_lookup(&remote, repo, "origin");
   CheckError("Remote is invalid");

   // Fetch from remotes
   git_fetch_options fetchOpts = GIT_FETCH_OPTIONS_INIT;
   fetchOpts.callbacks.transfer_progress = fetch_progress;
   fetchOpts.depth = 1;
   fetchOpts.prune = GIT_FETCH_PRUNE;
   error = git_remote_fetch(remote, nullptr, &fetchOpts, nullptr);
   if (error != GIT_ENOTFOUND && !refName.empty()) {
      // This error is valid only when first repo init
      CheckError("Remote fetch");
   }

   // Search for heads
   git_oid fetch_head_oid;
   error = git_repository_fetchhead_foreach(repo, fetchhead_callback, &fetch_head_oid);
   CheckError("Fetch head for each");

   // Checkout current head FORCE
   git_checkout_options checkoutOpts = GIT_CHECKOUT_OPTIONS_INIT;
   checkoutOpts.checkout_strategy = GIT_CHECKOUT_FORCE;
   error = git_checkout_head(repo, &checkoutOpts);
   // No head yet
   if (error == GIT_EUNBORNBRANCH) {
      // Get commit from fetched origin
      git_commit* commit;
      error = git_commit_lookup(&commit, repo, &fetch_head_oid);
      CheckError("Commit lookup");

      // Create branch from it
      git_reference* branch;
      error = git_branch_create(&branch, repo, "main", commit, true);
      CheckError("Create main brach");

      // Set head to it
      error = git_repository_set_head(repo, refName.c_str());
      CheckError("Set head new");

      git_reference_free(branch);
      git_commit_free(commit);

      // Checkout head to id
      error = git_checkout_head(repo, &checkoutOpts);
   }
   CheckError("Checkout head");

   error = git_repository_set_head(repo, refName.c_str());
   CheckError("Set head");

   git_remote_free(remote);
}


void ResetRepo(git_repository* repo, const char* repoPath)
{
   bool anyChanges = false;
   git_status_options statusopt = GIT_STATUS_OPTIONS_INIT;
   statusopt.show = GIT_STATUS_SHOW_INDEX_AND_WORKDIR;
   statusopt.flags = GIT_STATUS_OPT_INCLUDE_UNTRACKED;

   git_status_list* status;
   error = git_status_list_new(&status, repo, &statusopt);
   CheckError("Status List");

   size_t status_count = git_status_list_entrycount(status);
   for (size_t i = 0; i < status_count; ++i) {
      const git_status_entry* s = git_status_byindex(status, i);

      // Collect more states because remote fetch skipped checkout 
      if (s->status & (
         GIT_STATUS_WT_NEW | GIT_STATUS_WT_MODIFIED | GIT_STATUS_WT_DELETED |
         GIT_STATUS_WT_TYPECHANGE | GIT_STATUS_WT_RENAMED | GIT_STATUS_WT_UNREADABLE
         )) {
         ThrowIfNotWithAdminRights();
         anyChanges = true;
         break;
      }
   }

   git_status_list_free(status);

   if (anyChanges) {
      // Reset because RemoteFetch now skips force checkout
      git_object* head_obj = nullptr;
      error = git_revparse_single(&head_obj, repo, "HEAD");
      CheckError("Get Head");

      error = git_reset(repo, head_obj, GIT_RESET_HARD, nullptr);
      git_object_free(head_obj);
      CheckError("Reset");

      // Remove unversioned files
      error = git_status_list_new(&status, repo, &statusopt);
      CheckError("Status List");

      char long_path[4096];
      size_t repoPathLength = strlen(repoPath);
      memcpy(long_path, repoPath, repoPathLength + 1);
      long_path[repoPathLength] = '\\';

      status_count = git_status_list_entrycount(status);
      for (size_t i = 0; i < status_count; ++i) {
         const git_status_entry* s = git_status_byindex(status, i);
         if (s->status & GIT_STATUS_WT_NEW) {
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
}


void UpdateRepo(const char* repoPath, const char* remoteRepoUrl, const char* branchName) {
   git_repository* repo = nullptr;
   git_remote* remote = nullptr;

   // Try to open the repository
   error = git_repository_open(&repo, repoPath);
   if (error < 0) {
      InitRepo(&repo, &remote, repoPath, remoteRepoUrl);
   }

   // Once the repository is cloned or opened, perform the rest of the
   if (repo) {
      try {
         RemoteFetch(repo, remote, remoteRepoUrl, branchName);
         
         ResetRepo(repo, repoPath);
      }
      catch (admin_rights_exception ex) {
         if (repo) {
            git_repository_free(repo);
         }
         throw ex;
      }
      git_repository_free(repo);
   }
}


void LaunchAdminNanoUpdater() {
   std::filesystem::path cwd = std::filesystem::current_path();

   SHELLEXECUTEINFO ShExecInfo = { 0 };
   ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
   ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
   ShExecInfo.hwnd = NULL;
   ShExecInfo.lpVerb = "open";
   ShExecInfo.lpFile = "..\\Tools\\PW_NanoUpdaterAdm.exe";
   ShExecInfo.lpParameters = "";
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
   }
}


bool IsAdminRequired() {

   std::ofstream srrr("..\\Tools\\test");
   if (!srrr.fail()) {
      srrr.close();
      remove("..\\Tools\\test");
   }
   return srrr.fail();
}



#if 1
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
      return 1; // Exit program
   }

   isAdminRightsRequired = IsAdminRequired();
   std::ofstream errLogs;
   if (!isAdminRightsRequired) {
      errLogs.open(isRunningAdm ? "adm_upd_errors.txt" : "upd_errors.txt");
      std::cerr.rdbuf(errLogs.rdbuf());
   } else {
      std::cerr.rdbuf(new null_streambuf);
   }

   const char* repoPath[] = {
      "..\\Launcher\\content",
      //"..\\Game",
   };

   const char* repoLabels[] = {
      "content",
      //"game",
   };

   const char* remoteRepoUrl[] = {
      "https://gitlab.com/Rekongstor/content.git",
      //"https://gitlab.com/Rekongstor/PWCGitUpdates.git",
   };
   const char* branchName = "main";


   // Initialize the library
   int _error = git_libgit2_init();
   if (_error < 0) {
      std::cerr << "Failed to init libgit2 library!";
      return 0;
   }

#ifndef ADMIN_MANIFEST
   _error = git_libgit2_opts(GIT_OPT_SET_OWNER_VALIDATION, false);
   if (_error < 0) {
      std::cerr << "Failed to set non-admin opts for libgit2!";
      return 0;
   }
#endif

   try {
      for (int i = 0; i < _countof(repoPath); ++i) {
         if (isRunningAdm) {
            std::stringstream strStream;
            strStream << "#{\"type\":\"label\", \"data\":\"" << repoLabels[i] << "\"}" << std::endl;
            TransmitMessage(strStream.str().c_str(), strStream.str().size());
         }
         else {
            std::cout << "#{\"type\":\"label\", \"data\":\"" << repoLabels[i] << "\"}" << std::endl;
         }

         UpdateRepo(repoPath[i], remoteRepoUrl[i], branchName);
      }
   }
   catch (admin_rights_exception) {
#ifndef ADMIN_MANIFEST
      _error = git_libgit2_shutdown();
      LaunchAdminNanoUpdater();
      return 0;
#endif
   }
   catch (std::exception ex) {
      std::cerr << "Exception: " << ex.what();
      return 0;
   }

   _error = git_libgit2_shutdown();
   if (_error < 0) {
      std::cerr << "Failed to shutdown libgit2 library!";
   }

   return 0;

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

   for (int r = 0; r < _countof(releaseFileUrls); ++r) {
      if (isRunningAdm) {
         std::stringstream strStream;
         strStream << "#{\"type\":\"label\", \"data\":\"" << "game_data" << r << "\"}" << std::endl;
         TransmitMessage(strStream.str().c_str(), strStream.str().size());
      }
      else {
         std::cout << "#{\"type\":\"label\", \"data\":\"" << "game_data" << r << "\"}" << std::endl;
      }
      DownloadRelease(releaseFileUrls[r], releaseFilePaths[r], releaseFileHashes[r]);
   }

#ifdef ADMIN_MANIFEST
   doWork.store(false);
   writeStdOut.join();
#endif
   return 0;
}
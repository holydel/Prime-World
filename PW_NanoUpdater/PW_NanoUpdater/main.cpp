#include <windows.h>
#include <iostream>
#include <git2.h>

#include <locale>
#include <codecvt>
#include <string>


static const char* repoPath = ".\\Repo";
static const char* remoteRepoUrl = "https://github.com/Prime-World-Classic/PWCGitUpdates.git";
static const char* releaseFileNames[] = { "" };
static int error;

bool gh_fs_rm(const std::wstring& path) {
   WIN32_FIND_DATA findFileData;
   HANDLE hFind = INVALID_HANDLE_VALUE;

   // Добавляем к пути "\\*" для поиска всех файлов
   std::wstring strPath = path + L"\\*";

   hFind = FindFirstFile(strPath.c_str(), &findFileData);
   if (hFind == INVALID_HANDLE_VALUE) {
      return false;
   }

   do {
      const std::wstring fileName = findFileData.cFileName;
      const std::wstring fullPath = path + L"\\" + fileName;

      if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
         // Не удаляем текущую (".") и родительскую ("..")иректории
         if (fileName != L"." && fileName != L"..") {
            // Рекурсивно удаляем вложенные директории
            if (!gh_fs_rm(fullPath)) {
               FindClose(hFind);
               return false;
            }
         }
      }
      else {
         // Удаляем файл
         if (!DeleteFile(fullPath.c_str())) {
            std::cerr << "Error in DeleteFile: " << GetLastError() << std::endl;
            FindClose(hFind);
            return false;
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

void DownloadRelease(const std::string& releaseFileName) {
   // ... Add the code to download a release file here ...
}

void CheckError(const char* stage)
{
   if (error < 0) {
      std::cerr << stage << " failed: " << git_error_last()->message << std::endl;
      throw;
   }
}

// This is a callback that could be used to print fetch head information
int fetchhead_callback(const char* ref_name, const char* remote_url, const git_oid* oid, unsigned int is_merge, void* payload) {
   memcpy(payload, oid, sizeof(git_oid));
   return 0; // return-zero to stop iterating
}
// Вспомогательная функция для создания коммиталияния
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
   std::cout << "Received: " << stats->received_objects << "/" << stats->total_objects << " objects" << std::endl;
   std::cout << "Received: " << stats->received_bytes << "/" << stats->total_objects << " bytes" << std::endl;
   return 0;
}

void UpdateRepo(const char* branch) {
   git_repository* repo = nullptr;
   std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
   std::wstring wRepoPath = converter.from_bytes(repoPath);

   // Try to open the repository
   error = git_repository_open(&repo, repoPath);
   if (error) {
      // If opening the repository failed, delete the target directory

      // Assuming gh_fs_rm is a function that recursively deletes a directory.
      error = gh_fs_rm(wRepoPath);
      CheckError("Directory deletion");

      // Clone
      git_clone_options cloneOpts = GIT_CLONE_OPTIONS_INIT;
      //cloneOpts.fetch_opts.depth = 1;
      cloneOpts.fetch_opts.callbacks.transfer_progress = fetch_progress;
      
      error = git_clone(&repo, remoteRepoUrl, repoPath, &cloneOpts);
      CheckError("Clone");
   }

   // Once the repository is cloned or opened, perform the rest of the
   if (repo) {
      // Reset
      git_object* head_obj = nullptr;
      error = git_revparse_single(&head_obj, repo, "HEAD");
      CheckError("Get Head");
      error = git_reset(repo, head_obj, GIT_RESET_HARD, nullptr);
      git_object_free(head_obj);
      CheckError("Reset");


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
      
      git_remote* remote = nullptr;
      error = git_remote_lookup(&remote, repo, "origin");
      CheckError("Remote lookup");

      git_fetch_options fetch_opts = GIT_FETCH_OPTIONS_INIT;
      fetch_opts.callbacks.transfer_progress = fetch_progress;
      error = git_remote_fetch(remote, nullptr, &fetch_opts, nullptr);
      CheckError("Remote fetch");

      // Verify if we actually fetched any new data
      git_oid fetch_head_oid;
      error = git_repository_fetchhead_foreach(repo, fetchhead_callback, &fetch_head_oid);
      CheckError("Fetching FETCH_HEAD");

      git_annotated_commit* fetched_commit = NULL;
      error = git_annotated_commit_from_fetchhead(&fetched_commit, repo, "main", remoteRepoUrl, &fetch_head_oid);
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
      } else {
         /*
         gh_fs_rm(wRepoPath);
         error = -1;
         CheckError("Failed to merge");
         */
      }

      // Cleanup
      git_remote_free(remote);
      git_repository_free(repo);
   }
   else {
      std::cerr << "Repository failed to initialize.\n";
      return;
   }
}

void Update(bool skipReleaseDownload) {

   // Initialize the library
   int _error = git_libgit2_init();
   if (_error < 0) {
      std::cerr << "Failed to init libgit2 library!";
      return;
   }

   try {
      UpdateRepo("main");
      // UpdateRepo("launcher");
   }
   catch (std::exception ex) {
      std::cerr << "Exception: " << ex.what();
   }

   _error = git_libgit2_shutdown();
   if (_error < 0) {
      std::cerr << "Failed to shutdown libgit2 library!";
      return;
   }
}

int main(){
   Update(false);
}
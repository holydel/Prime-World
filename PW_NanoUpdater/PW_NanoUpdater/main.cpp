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

void UpdateRepo(const char* branch) {
   git_repository* repo = nullptr;

   // Try to open the repository
   error = git_repository_open(&repo, repoPath);
   if (error) {
      // If opening the repository failed, delete the target directory
      std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
      std::wstring wRepoPath = converter.from_bytes(repoPath);

      // Assuming gh_fs_rm is a function that recursively deletes a directory.
      error = gh_fs_rm(wRepoPath);
      CheckError("Directory deletion");

      // Clone
      git_clone_options cloneOpts = GIT_CLONE_OPTIONS_INIT;
      cloneOpts.fetch_opts.depth = 1;
      
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
      error = git_remote_fetch(remote, nullptr, &fetch_opts, nullptr);
      CheckError("Remote fetch");

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
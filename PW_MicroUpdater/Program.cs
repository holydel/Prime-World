using System;
using System.IO;
using System.Diagnostics;
using System.Threading;
using System.Security.Principal;
using LibGit2Sharp;
using System.Text;
using System.Runtime.InteropServices;
using System.Net;

namespace PW_MicroUpdater
{

   class Program
   {
      static string repoPath = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, @"..\\Game\\");
      static string targetBranch = @"main";
      static string remoteRepoUrl = "https://github.com/rekongstor/PWCGitUpdates.git";
      static bool IS_ADMIN_MANIFEST = false;
      static string releaseUrl = @"https://github.com/ip7z/7zip/archive/refs/tags";
      static string releaseFileName = @"24.09.zip";

      [DllImport("wininet.dll", SetLastError = true)]
      public static extern IntPtr InternetOpen(string lpszAgent, int dwAccessType, string lpszProxyName, string lpszProxyBypass, int dwFlags);

      [DllImport("wininet.dll", SetLastError = true)]
      public static extern IntPtr InternetOpenUrl(IntPtr hInternet, string lpszUrl, string lpszHeaders, int dwHeadersLength, int dwFlags, int dwContext);

      [DllImport("wininet.dll", SetLastError = true)]
      public static extern bool InternetReadFile(IntPtr hFile, byte[] lpBuffer, int dwNumberOfBytesToRead, out int lpdwNumberOfBytesRead);

      [DllImport("wininet.dll", SetLastError = true)]
      public static extern bool InternetCloseHandle(IntPtr hInternet);

      [DllImport("kernel32.dll")]
      static extern IntPtr GetConsoleWindow();

      [DllImport("user32.dll")]
      static extern bool ShowWindow(IntPtr hWnd, int nCmdShow);

      const int SW_HIDE = 0;

      static int GetReleaseSize(string fileUrl)
      {
         var currentSecurityProtocol = ServicePointManager.SecurityProtocol;
         ServicePointManager.SecurityProtocol = (SecurityProtocolType)(0x00000C00);
         string fileSize = "0";
         try
         {

            // Создание запроса к URL файла
            HttpWebRequest request = (HttpWebRequest)WebRequest.Create(fileUrl);
            request.Method = "HEAD"; // Иользуем метод HEAD, чтобы не скачивать содержимое

            using (HttpWebResponse response = (HttpWebResponse)request.GetResponse())
            {
               // Проверяем, есть ли заголовок Content-Length
               if (response.Headers["Content-Length"] != null)
               {
                  fileSize = response.Headers["Content-Length"];
               }
               else
               {
                  Console.WriteLine("File size request failed");
               }
            }
         }
         catch (Exception ex)
         {
            Console.WriteLine("Filed to get file size: " + ex.Message);
         }
         ServicePointManager.SecurityProtocol = currentSecurityProtocol;
         return Int32.Parse(fileSize);
      }

      static void DownloadRelease()
      {
         try
         {
            string filename = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, releaseFileName);

            string url = releaseUrl + "/" + releaseFileName;
            StringBuilder sb = new StringBuilder();

            IntPtr hInternet = InternetOpen("PWClassic", 0, null, null, 0);
            if (hInternet != IntPtr.Zero)
            {
               IntPtr hUrl = InternetOpenUrl(hInternet, url, null, 0, 0, 0);
               if (hUrl != IntPtr.Zero)
               {
                  int fileSize = GetReleaseSize(url);
                  int totalBytesRead = 0;

                  byte[] buffer = new byte[4096];
                  int bytesRead;

                  using (FileStream fs = new FileStream(filename, FileMode.Create))
                  { 
                     while (InternetReadFile(hUrl, buffer, buffer.Length, out bytesRead) && bytesRead > 0)
                     {
                        totalBytesRead += bytesRead;
                        double totalProgress = (float)totalBytesRead / (float)fileSize * 100.0;
                        Console.WriteLine($"{totalProgress:f1}%");
                        fs.Write(buffer, 0, bytesRead);
                     }
                  }

                  InternetCloseHandle(hUrl);
               }

               InternetCloseHandle(hInternet);
            }
         }
         catch (Exception e)
         {
            Console.WriteLine("Release download failed: " + e.Message);
         }
      }

      static void UnzipRelease()
      {

      }

      static bool IsDirectoryWritable(bool throwIfFails = false)
      {
         try
         {
            FileStream fs = File.Create(Path.Combine(AppDomain.CurrentDomain.BaseDirectory, 
               Path.GetRandomFileName()), 1, FileOptions.DeleteOnClose);
            return true;
         }
         catch
         {
            if (throwIfFails)
               throw;
            else
               return false;
         }
      }

      public static bool Pull(string repositoryPath)
      {
         Repository localRepo = new Repository(repositoryPath);
         PullOptions pullOptions = new PullOptions();
         pullOptions.FetchOptions = new FetchOptions();
         
         Commands.Pull(localRepo, new Signature("username", "<your email>", new DateTimeOffset(DateTime.Now)), pullOptions);
         return true;
      }

      static void UpdateGame()
      {
         try
         {
            var repo = new Repository(repoPath);
         }
         catch
         {
            try
            {
               if (System.IO.Directory.Exists(repoPath))
               {
                  System.IO.Directory.Delete(repoPath, true);
               }
               var result = Repository.Clone(remoteRepoUrl, repoPath);
            }
            catch (Exception e2)
            {
               Console.WriteLine("Clone failed: " + e2.Message);
            }
         }
         try
         {
            using (var repo = new Repository(repoPath))
            {
               try
               {
                  Commands.Stage(repo, "*");
               }
               catch (Exception e)
               {
                  Console.WriteLine("Stage failed: " + e.Message);
               }
               try
               {
                  //var trackedBranch = repo.Head.TrackedBranch;
                  //Commit originHeadCommit = repo.ObjectDatabase.FindMergeBase(repo.Branches[targetBranch].Tip, repo.Head.Tip);
                  repo.Reset(LibGit2Sharp.ResetMode.Hard, repo.Branches[targetBranch].Tip);

               }
               catch (Exception e)
               {
                  Console.WriteLine("Reset failed: " + e.Message);
               }

               try
               {
                  LibGit2Sharp.Commands.Checkout(repo, targetBranch);
               }
               catch (Exception e)
               {
                  Console.WriteLine("Checkout failed: " + e.Message);
               }

               try
               {
                  Pull(repoPath);
               }
               catch (Exception e)
               {
                  Console.WriteLine("Pull failed: " + e.Message);
               }

               /*
               File.WriteAllText(Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "update.ini"),
                  repo.Branches[targetBranch].Tip.Id.Sha);
               */
               DownloadRelease();
               //return;
            }
         }
         catch (Exception e)
         {
            Console.WriteLine("Everything failed: " + e.Message);
         }
      }

      static void Main(string[] args)
      {
         var handle = GetConsoleWindow();

         // Hide
         ShowWindow(handle, SW_HIDE);

         if (IsDirectoryWritable())
         {
            UpdateGame();
         } else
         {
            if (IS_ADMIN_MANIFEST)
            {
               try
               {
                  IsDirectoryWritable(true);
               }
               catch (Exception e)
               {
                  WindowsPrincipal myPrincipal = (WindowsPrincipal)Thread.CurrentPrincipal;
                  if (myPrincipal.IsInRole(WindowsBuiltInRole.Administrator))
                  {
                     Console.WriteLine("Ошибка обновления! Обратитесь в поддержку PWClassic https://t.me/primeworldclassic/8232 \n" + e.Message);
                     return;
                  }
                  else
                  {
                     Console.WriteLine("Запустите приложение от имени администратора!");
                     return;
                  }
               }
            } else {
               ProcessStartInfo startInfo = new ProcessStartInfo
               {
                  FileName = AppDomain.CurrentDomain.BaseDirectory + "PW_MicroUpdaterA.exe",
                  Arguments = "",
                  UseShellExecute = true,
                  WorkingDirectory = AppDomain.CurrentDomain.BaseDirectory
               };
               Process.Start(startInfo);
            }
         }

      }
   }
}

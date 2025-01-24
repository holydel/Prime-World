using System;
using System.IO;
using System.Diagnostics;
using System.Threading;
using System.Security.Principal;
using LibGit2Sharp;
using System.Text;
using System.Runtime.InteropServices;
using System.Net;
using System.Security.Cryptography;
using System.Linq;

namespace PW_MicroUpdater
{

   class Program
   {
      static string repoPath = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "..\\Game\\");
      static string pileFilesPath = Path.Combine(repoPath, "Packs\\");
      static string targetBranch = "main";
      static string remoteRepoUrl = "https://github.com/Prime-World-Classic/PWCGitUpdates.git";
      static string releaseUrl = "https://github.com/Prime-World-Classic/Prime-World/releases/download/1.9";
      static string[] releaseFileNames = {
         "data01.pile",
         "data02.pile",
         "data03.pile",
         "data04.pile",
         "data05.pile",
         "data06.pile",
      };


      static bool IS_ADMIN_MANIFEST = false;
      static bool COMPUTE_HASH = false;

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

      private static bool CheckFileMD5Hash(string filename, string md5FileName)
      {
         if (File.Exists(filename))
         {
            using (var md5 = MD5.Create())
            {
               using (var stream = File.OpenRead(filename))
               {
                  var hash = md5.ComputeHash(stream);
                  var validHash = System.IO.File.ReadAllBytes(md5FileName);

                  return hash.SequenceEqual(validHash);
               }
            }
         }
         return false;
      }

      private static void GenerateFileMD5Hash(string filename, string md5FileName)
      {
         using (var md5 = MD5.Create())
         {
            using (var stream = File.OpenRead(filename))
            {
               var hash = md5.ComputeHash(stream);
               System.IO.File.WriteAllBytes(md5FileName, hash);
               return;
            }
         }
      }

      static void DownloadRelease(string releaseFileName)
      {
         try
         {

            string filename = Path.Combine(pileFilesPath, releaseFileName);
            string md5FileName = Path.Combine(pileFilesPath, "..\\Hashes\\" + releaseFileName + ".md5");

            if (COMPUTE_HASH)
            {
               GenerateFileMD5Hash(filename, md5FileName);
               return;
            } else
            {
               if (CheckFileMD5Hash(filename, md5FileName))
               {
                  return;
               }
            }

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

            if (CheckFileMD5Hash(filename, md5FileName))
            {
               Console.WriteLine("Failed to update: " + releaseFileName);
            }
         }
         catch (Exception e)
         {
            Console.WriteLine("Release download failed: " + e.Message);
         }
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

      static void UpdateGame(bool skipReleaseDownload)
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
                  Repository localRepo = new Repository(repoPath);
                  PullOptions pullOptions = new PullOptions();
                  pullOptions.FetchOptions = new FetchOptions();

                  Commands.Pull(localRepo, new Signature("username", "<your email>", new DateTimeOffset(DateTime.Now)), pullOptions);
               }
               catch (Exception e)
               {
                  Console.WriteLine("Pull failed: " + e.Message);
               }

               if (!skipReleaseDownload)
               {
                  foreach (string releaseFileName in releaseFileNames)
                  {
                     DownloadRelease(releaseFileName);
                  }
               }
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

         bool skipReleaseDownload = false;
         if (args.Length > 0)
         {
            foreach (string arg in args)
            {
               if (arg == "skip_release")
               {
                  skipReleaseDownload = true; break;
               }
            }
         }

         // Hide
         ShowWindow(handle, SW_HIDE);

         if (IsDirectoryWritable())
         {
            UpdateGame(skipReleaseDownload);
         }
         else
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
            }
            else
            {
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

         //Console.ReadKey();
      }
   }
}

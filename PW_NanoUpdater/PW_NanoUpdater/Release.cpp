#include <Windows.h>
#include <wininet.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <strstream>
#include <iostream>
#include <format>

#include <filesystem>

#pragma comment(lib, "Wininet.lib")

extern bool isRunningAdm;
class admin_rights_exception : std::exception {};
class unhandled_error : std::exception {};

void ThrowIfNotWithAdminRights();
void TestKillAdminProcess();


void TransmitMessage(char const* strbuf, std::streamsize strSize);


int GetReleaseSize(const std::string& fileUrl)
{
   DWORD certopt = 0x00000C00;
   InternetSetOption(NULL, INTERNET_OPTION_SECURITY_SELECT_CLIENT_CERT, (LPVOID)&certopt, sizeof(DWORD));

   std::string fileSize = "0";

   HINTERNET hInternet = InternetOpen("UserAgent", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
   if (!hInternet)
   {
      std::cerr << "Failed to open Internet" << std::endl;
      return 0;
   }

   HINTERNET hConnect = InternetOpenUrlA(hInternet, fileUrl.c_str(), NULL, 0, INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_NO_COOKIES | INTERNET_FLAG_NO_UI, 0);
   if (!hConnect)
   {
      std::cerr << "Failed to open URL" << std::endl;
      InternetCloseHandle(hInternet);
      return 0;
   }

   DWORD dwContentLength = 0;
   DWORD dwBufferLength = sizeof(DWORD);
   HttpQueryInfo(hConnect, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &dwContentLength, &dwBufferLength, NULL);
   fileSize = std::to_string(dwContentLength);

   InternetCloseHandle(hConnect);
   InternetCloseHandle(hInternet);

   return std::stoi(fileSize);
}


bool CheckFileMD5Hash(const std::string& filename, const std::string& md5FileName) {
   std::ifstream fileStream(filename, std::ios::binary);
   std::ifstream md5File(md5FileName, std::ios::binary);

#ifdef NDEBUG
   if (md5File.fail()) {
      std::cerr << "Hash failed" << std::endl;
#ifdef TEST_HASHES
      return false;
#else
      return true;
#endif
   }
#endif
   if (fileStream.is_open()) {
      HCRYPTPROV hProv;
      HCRYPTHASH hHash;

      if (CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
         if (CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash)) {
            const DWORD BufferSize = 4096;
            std::vector<BYTE> buffer(BufferSize);
            DWORD bytesRead = 0;

            while (fileStream.read(reinterpret_cast<char*>(buffer.data()), BufferSize) && fileStream.gcount() > 0) {
               CryptHashData(hHash, buffer.data(), fileStream.gcount(), 0);
            }

            DWORD hashSize;
            DWORD outSize;
            outSize = sizeof(DWORD);

            CryptGetHashParam(hHash, HP_HASHSIZE, reinterpret_cast<BYTE*>(&hashSize), &outSize, 0);

            std::vector<BYTE> hashData(hashSize);

            CryptGetHashParam(hHash, HP_HASHVAL, hashData.data(), &hashSize, 0);

            CryptDestroyHash(hHash);
            CryptReleaseContext(hProv, 0);

            bool isValid = false;
            if (md5File.is_open()) {
               std::vector<BYTE> validHash((std::istreambuf_iterator<char>(md5File)), std::istreambuf_iterator<char>());
               isValid = hashData == validHash;
            }
#ifndef NDEBUG
            md5File.close();
            std::ofstream md5FileOut(md5FileName, std::ios::binary);
            md5FileOut.write(reinterpret_cast<char*>(hashData.data()), hashData.size());
            return true;
#endif
            return isValid;
         }
      }
   }

   return false;
}


void DownloadRelease(const std::string& fileUrl, const std::string& filePath, const std::string& md5Path, int r)
{
   try
   {
      std::string filename = filePath;

      if (CheckFileMD5Hash(filePath, md5Path)) {
         return;
      }

      ThrowIfNotWithAdminRights();

      if (isRunningAdm) {
         std::stringstream strStream;
         strStream << "#{\"type\":\"label\", \"data\":\"" << "game_data" << r << "\"}" << std::endl;
         TransmitMessage(strStream.str().c_str(), strStream.str().size());
      }
      else {
         std::cout << "#{\"type\":\"label\", \"data\":\"" << "game_data" << r << "\"}" << std::endl;
      }

      HINTERNET hInternet = InternetOpen("PWClassic", 0, NULL, NULL, 0);
      if (hInternet)
      {
         HINTERNET hUrl = InternetOpenUrl(hInternet, fileUrl.c_str(), NULL, 0, 0, 0);
         if (hUrl)
         {
            int fileSize = GetReleaseSize(fileUrl);
            int totalBytesRead = 0;
            BYTE buffer[4096];
            DWORD bytesRead;

            std::ofstream fs(filename, std::ios::binary);
            if (fs.is_open())
            {
               while (InternetReadFile(hUrl, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0)
               {
                  TestKillAdminProcess();
                  totalBytesRead += bytesRead;
                  int progress = min(100, max(0, (int)((float)totalBytesRead / (float)fileSize * 100.0)));         
                  if (isRunningAdm) {
                     std::stringstream strStream;
                     strStream << "#{\"type\":\"bar\", \"data\":\"" << progress << "\"}" << std::endl;
                     TransmitMessage(strStream.str().c_str(), strStream.str().size());
                  } else {
                     std::cout << "#{\"type\":\"bar\", \"data\":\"" << progress << "\"}" << std::endl;
                  }
                  fs.write(reinterpret_cast<char*>(buffer), bytesRead);
               }
               fs.close();
            }

            InternetCloseHandle(hUrl);
         }

         InternetCloseHandle(hInternet);
      }

      if (!CheckFileMD5Hash(filename, md5Path)) {
         std::cerr << "Failed to update: " << filename << std::endl;
         throw unhandled_error();
      }
   }
   catch (admin_rights_exception ex) {
      throw ex;
   }
   catch (const std::exception& e) {
      std::cerr << "Release download failed: " << e.what() << std::endl;
      throw e;
   }
}
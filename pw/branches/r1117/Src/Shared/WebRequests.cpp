
#include "WebRequests.h"

bool useMirrorServer = false;

#pragma comment(lib, "wininet.lib")


WebPostRequest::WebPostRequest(const wchar_t* serverUrl, const wchar_t* objectName, int serverPort, DWORD flags)
{
  Init(serverUrl, objectName, serverPort, flags);
}


void WebPostRequest::Init(const wchar_t* serverUrl, const wchar_t* objectName, int serverPort, DWORD flags)
{
  const std::wstring apiUrl = serverUrl;

  //const std::string apiEndpoint = "/api/launcher/";

  hInternet = InternetOpenW(L"HTTP Post Request", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
  if (hInternet == NULL) {
    return;
  }

  // Connect to the server
  hConnect = InternetConnectW(hInternet, apiUrl.c_str(), serverPort, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
  if (hConnect == NULL) {
    InternetCloseHandle(hInternet);
    return;
  }

  // Open the HTTP request
  hRequest = HttpOpenRequestW(hConnect, L"POST", objectName, NULL, NULL, NULL, flags, 0);
  if (hRequest == NULL) {
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);
    return;
  }
}

WebPostRequest::~WebPostRequest()
{
  // Close handles
  InternetCloseHandle(hRequest);
  InternetCloseHandle(hConnect);
  InternetCloseHandle(hInternet);
}

std::string WebPostRequest::SendPostRequest(const std::string& jsonData) {
  // Set headers and data for the POST request
  const char* headers = "Content-Type: application/json\r\n";
  const char* postData = jsonData.c_str();
  DWORD postDataLen = jsonData.length();
  DWORD headersDataLen = strlen(headers);

  // Send the HTTP request
  BOOL bRequestSent = HttpSendRequestA(hRequest, headers, headersDataLen, (LPVOID)postData, postDataLen);
  if (!bRequestSent) {
    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);
    return "";
  }

  // Read the response
  char buffer[4096];
  DWORD bytesRead = 0;
  std::string responseStream;

  while (InternetReadFile(hRequest, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead > 0) {
    buffer[bytesRead] = '\0'; // Null-terminate the buffer
    responseStream += buffer;
  }

  OutputDebugStringA(responseStream.c_str());

  return responseStream;
}

std::string GetSessionData(const char* token) {
  WebPostRequest request(SERVER_IP_W, L"/api", SYNCHRONIZER_PORT, 0);

  Json::Value data;
  data["sessionToken"] = Json::Value (std::string(token, 32));
  data["apiKey"] = Json::Value (API_KEY);

  Json::Value result;
  result["data"] = data;
  result["method"] = Json::Value("createWebSession");

  Json::FastWriter writer;
  std::string res = writer.write(result);

  return request.SendPostRequest(res);
}
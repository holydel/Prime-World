#pragma once
#include <vector>
#include <Windows.h>
#include <Wininet.h>
#include <map>
#include <string>
#include <set>
#include <json/json.h>
#include "../PW_Game/server_ip.h"

class WebLauncherPostRequest
{
	HINTERNET hInternet;
	HINTERNET hConnect;
	HINTERNET hRequest;

	std::map<std::string, int> characterMap;
	std::map<int, std::string> classTalentMap;

	std::vector<int> keysClassTalent;
public:
  WebLauncherPostRequest();
	WebLauncherPostRequest(const wchar_t* serverUrl, const wchar_t* objectName, int serverPort, DWORD flags);

  void Init(const wchar_t* serverUrl, const wchar_t* objectName, int serverPort, DWORD flags);


  void FillData();

  ~WebLauncherPostRequest();

  enum LoginResponse {
    LoginResponse_OK,
    LoginResponse_FAIL,
    LoginResponse_BLOCK,
    LoginResponse_OFFLINE, // Not safe

    LoginResponse_WEB_CREATE,
    LoginResponse_WEB_CONNECT,
    LoginResponse_WEB_RECONNECT,
    LoginResponse_WEB_FAIL,
  };

  struct WebLoginResponse {
    std::string response;
    LoginResponse retCode;
  };

  struct TalentWebData {
    int webTalentId;
    int activeSlot; // negative = smart cast
    bool isSmartCast;
  };

  struct WebUserData {
    WebUserData(): heroSkinID(0), currentRating(1100), victoryRating(1100), lossRating(1100) {}
    std::vector<TalentWebData> talents;
	  int heroSkinID;
    float currentRating;
    float victoryRating;
    float lossRating;
  };

  struct PlayerInfoByUserId {
    wstring nickname;
    int teamId;
    bool isLeaver;
  };

  enum RegisterSessionRequest {
    RegisterInSessionRequest_Create,
    RegisterInSessionRequest_Wait,
    RegisterInSessionRequest_Connect,

    RegisterInSessionRequest_Reconnect,

    RegisterInSessionRequest_Joined,
    RegisterInSessionRequest_HeroSelected,
    RegisterInSessionRequest_InReadyState,

    RegisterInSessionRequest_WebCreate,
    RegisterInSessionRequest_WebConnect,
    RegisterInSessionRequest_WebReconnect,

    RegisterInSessionRequest_WebJoined,
    RegisterInSessionRequest_WebHeroSelected,

    RegisterInSessionRequest_Error,
  };

	std::vector<TalentWebData> GetTallentSet(const wchar_t* nickName, const char* heroName);
  std::map<std::wstring, WebUserData> WebLauncherPostRequest::GetUsersData(const std::vector<std::wstring>& nickNames, const std::vector<std::string>& heroNames);
  std::map<std::wstring, WebUserData> WebLauncherPostRequest::GetLegacyUsersData(const std::vector<std::wstring>& nickNames, const std::vector<std::string>& heroNames);
  std::string ConvertFromClassID(int id);
  WebLoginResponse GetSessionData(const char* token, const char* apiKey = "");
  WebLoginResponse GetNickName(const char* token);
  std::string WebLauncherPostRequest::SendPostRequest(const std::string& jsonData);
  RegisterSessionRequest RegisterInSession(const char* nickname, int heroId, const char* sessionToken, string& gameName);
  RegisterSessionRequest ReconnectInSession(const char* sessionToken, string& gameName);
  void LobbyCreatedRequest(const char* nickname, const char* sessionToken);
  bool CheckIsGameReady(const char* sessionToken);
  bool CheckConnectionRequest();
  void ValidateInstallationRequest(const char* playerToken);
  void SendSessionResults(const vector<int>& playerUserIds, int winningTeam);
  void SendFinishGameRequest(const vector<int>& playerUserIds, int winningTeam);
  std::string CreateDebugSession();
  void GetGameNameForConnection(const char* token);
  void NotifyGameStart(const char* nickname, const char* sessionToken);
};

extern std::string GetSkinByHeroPersistentId(const std::string& heroId, int someValue);

static std::string WideCharToMultiByteString(const wchar_t* wideCharString) {
  int size_needed = WideCharToMultiByte(CP_UTF8, 0, wideCharString, -1, NULL, 0, NULL, NULL);
  std::string result(size_needed, 0);
  WideCharToMultiByte(CP_UTF8, 0, wideCharString, -1, &result[0], size_needed, NULL, NULL);
  return result;
}
static std::string Fix1251Encoding(std::string utf8String)
{
  int utf8Length = static_cast<int>(utf8String.length());
  int wideCharLength = MultiByteToWideChar(CP_UTF8, 0, utf8String.c_str(), utf8Length, NULL, 0);

  wchar_t* wideCharString = new wchar_t[wideCharLength + 1];
  MultiByteToWideChar(CP_UTF8, 0, utf8String.c_str(), utf8Length, wideCharString, wideCharLength);
  wideCharString[wideCharLength] = L'\0';

  int win1251Length = WideCharToMultiByte(1251, 0, wideCharString, -1, NULL, 0, NULL, NULL);
  char* win1251String = new char[win1251Length];
  WideCharToMultiByte(1251, 0, wideCharString, -1, win1251String, win1251Length, NULL, NULL);

  return win1251String;
}

static Json::Value ParseJson(const char* json) {
  Json::Reader jsonReader;
  Json::Value root;
  bool isOk = jsonReader.parse(json, root, false);
  return isOk ? root : Json::Value();
}
#pragma once
#include <vector>
#include <Windows.h>
#include <Wininet.h>
#include <map>
#include <string>

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
  WebLoginResponse GetSessionData(const char* token);
  WebLoginResponse GetNickName(const char* token);
  std::string WebLauncherPostRequest::SendPostRequest(const std::string& jsonData);
  RegisterSessionRequest RegisterInSession(const char* nickname, int heroId, const char* sessionToken, string& gameName);
  RegisterSessionRequest ReconnectInSession(const char* sessionToken, string& gameName);
  void LobbyCreatedRequest(const char* nickname, const char* sessionToken);
  bool CheckIsGameReady(const char* sessionToken);
  bool CheckConnectionRequest(const char* playerToken);
  void ValidateInstallationRequest(const char* playerToken);
  void SendSessionResults(const vector<int>& playerUserIds, int winningTeam);
  void SendFinishGameRequest(const vector<int>& playerUserIds, int winningTeam);
  std::string CreateDebugSession();
  void GetGameNameForConnection(const char* token);
  void NotifyGameStart(const char* nickname, const char* sessionToken);
};

extern std::string GetSkinByHeroPersistentId(const std::string& heroId, int someValue);
extern std::string WideCharToMultiByteString(const wchar_t* wideCharString);
extern std::string Fix1251Encoding(std::string utf8String);
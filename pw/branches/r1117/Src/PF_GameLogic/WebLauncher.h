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
    LoginResponse_OFFLINE, // Not safe
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
    std::vector<TalentWebData> talents;
  };

  struct PlayerInfoByUserId {
    wstring nickname;
    int teamId;
  };

  enum RegisterSessionRequest {
    RegisterInSessionRequest_Create,
    RegisterInSessionRequest_Wait,
    RegisterInSessionRequest_Connect,

    RegisterInSessionRequest_Reconnect,

    RegisterInSessionRequest_Joined,
    RegisterInSessionRequest_HeroSelected,
    RegisterInSessionRequest_InReadyState,

    RegisterInSessionRequest_Error,
  };

	std::vector<TalentWebData> GetTallentSet(const wchar_t* nickName, const char* heroName);
  std::map<std::wstring, WebUserData> WebLauncherPostRequest::GetUsersData(const std::vector<std::wstring>& nickNames, const std::vector<std::string>& heroNames);
	std::string ConvertFromClassID(int id);
  WebLoginResponse GetNickName(const char* token);
  std::string WebLauncherPostRequest::SendPostRequest(const std::string& jsonData);
  RegisterSessionRequest RegisterInSession(const char* nickname, int heroId, const char* sessionToken, string& gameName);
  RegisterSessionRequest ReconnectInSession(const char* sessionToken, string& gameName);
  void LobbyCreatedRequest(const char* nickname, const char* sessionToken);
  bool CheckIsGameReady(const char* sessionToken);
  bool CheckConnectionRequest();
  void ValidateInstallationRequest(const char* playerToken);
  void SendSessionResults(const vector<int>& playerUserIds, int winningTeam);
};

extern std::string GetSkinByHeroPersistentId(const std::string& heroId, int someValue);
extern std::string WideCharToMultiByteString(const wchar_t* wideCharString);
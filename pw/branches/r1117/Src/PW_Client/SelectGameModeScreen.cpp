#include "stdafx.h"
#include "SelectGameModeScreen.h"

#include "Core/CoreFSM.h"
#include "SelectGameModeLogic.h"
#include "PF_GameLogic/GameMaps.h"
#include "NewLobbyClientPW.h"

#include "System/InlineProfiler.h"
#include "../PF_GameLogic/WebLauncher.h"

extern string g_sessionName;
extern WebLauncherPostRequest::RegisterSessionRequest g_registerInSessionResponse;

namespace NGameX
{


SelectGameModeScreen::SelectGameModeScreen( Game::IGameContextUiInterface * _ctx ) :
gameCtx( _ctx )
{ 
}


#pragma optimize("", off)
bool SelectGameModeScreen::Init( UI::User * uiUser )
{ 
  NI_PROFILE_FUNCTION

  logic = new UI::SelectGameModeLogic;
  SetLogic( logic );
  logic->SetUser( uiUser );
  logic->SetScreen( this );
  logic->LoadScreenLayout( "Lobby_SelectGameMode" );

  {
    NI_PROFILE_BLOCK( "Maps" )
    StrongMT<Game::IGameContextUiInterface> locked = gameCtx.Lock();
    if ( !locked )
      return false;

    NWorld::IMapCollection * maps = locked->Maps();
    NDb::Ptr<NDb::MapList> pMapList = NDb::Get<NDb::MapList>( NDb::DBID( "\\Tech\\Default\\_.MAPLST.xdb" ) );

    NI_VERIFY( IsValid( pMapList ), "\\Tech\\Default\\_.MAPLST does not exist", return false )

    maps->InitCustomList(pMapList);

    for ( int i = 0; i < pMapList->maps.size(); ++i )
      logic->AddMapEntry( i, maps->CustomDescId( i ), maps->CustomTitle( i ), maps->CustomDescription( i ) );
  }

  if ( StrongMT<Game::IGameContextUiInterface> cl = gameCtx.Lock() )
    cl->RefreshGamesList();

  return true; 
} 


void SelectGameModeScreen::Step( bool bAppActive )
{
  StrongMT<Game::IGameContextUiInterface> locked = gameCtx.Lock();
  if ( !locked || !logic )
    return;

  lobby::TDevGamesList infos;
  locked->PopGameList( infos );

  if (g_registerInSessionResponse == WebLauncherPostRequest::RegisterInSessionRequest_Create) {
    locked->CreateGame("Maps/Multiplayer/MOBA/_.ADMPDSCR.xdb", 10);
    g_registerInSessionResponse = WebLauncherPostRequest::RegisterInSessionRequest_Joined;
  }
  

  for( lobby::TDevGamesList::iterator it = infos.begin(); it != infos.end(); ++it )
    logic->UpdateSessionInfo( *it );


  if (g_registerInSessionResponse == WebLauncherPostRequest::RegisterInSessionRequest_Connect) {
    int requiredGameId = -1;

    // Fix 1251 encoding
    int utf8Length = static_cast<int>(g_sessionName.length());
    int wideCharLength = MultiByteToWideChar(CP_UTF8, 0, g_sessionName.c_str(), utf8Length, NULL, 0);

    wchar_t* wideCharString = new wchar_t[wideCharLength + 1];
    MultiByteToWideChar(CP_UTF8, 0, g_sessionName.c_str(), utf8Length, wideCharString, wideCharLength);
    wideCharString[wideCharLength] = L'\0';

    wstring nameTofind = wideCharString;
    nameTofind += L"'s game";

    for( lobby::TDevGamesList::iterator it = infos.begin(); it != infos.end(); ++it ) {
      OutputDebugStringW(it->name.c_str());
      if (nameTofind.compare(it->name.c_str() + 1) == 0) {
        requiredGameId = it->gameId;
      }
    }
    if (requiredGameId != -1) {
      locked->JoinGame(requiredGameId);
      g_registerInSessionResponse = WebLauncherPostRequest::RegisterInSessionRequest_Joined;
    }
  }


  lobby::EOperationResult::Enum joinResult = locked->LastLobbyOperationResult();
  if ( joinResult != lobby::EOperationResult::InProgress )
    logic->UpdateJoinResult( joinResult );

  DefaultScreenBase::Step( bAppActive );
}

} //namespace NGameX

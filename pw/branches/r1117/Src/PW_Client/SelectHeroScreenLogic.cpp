#include "stdafx.h"

#include "UI/Button.h"
#include "UI/RadioPanel.h"

#include "SelectHeroScreen.h"
#include "Render/TextureManager.h"

#include "SelectHeroScreenLogic.h"
#include "NewLobbyClientPW.h"
#include "Client/ScreenCommands.h"

extern string g_devLogin;
extern bool g_needNotifyLobbyClients;

namespace UI
{

#pragma warning(push)
#pragma warning(disable:4686)
BEGIN_LUA_TYPEINFO( SelectHeroScreenLogic, ClientScreenUILogicBase )
  LUA_METHOD( SelectHero )
  LUA_METHOD( PlayerReady )
  LUA_METHOD( LeaveLobby )
  LUA_METHOD( ChangeTeam )
  LUA_METHOD( ChangeFaction )
  LUA_METHOD( SetDeveloperParty )
END_LUA_TYPEINFO( SelectHeroScreenLogic )
#pragma warning(pop)
  

void SelectHeroScreenLogic::SelectHero( const char * heroId )
{
  if ( StrongMT<Game::IGameContextUiInterface> locked = screen->GameCtx().Lock() )
    locked->ChangeCustomGameSettings( lobby::ETeam::None, lobby::ETeam::None, heroId );
}



void SelectHeroScreenLogic::PlayerReady()
{ 
  isPlayerReady = true;
  if ( StrongMT<Game::IGameContextUiInterface> locked = screen->GameCtx().Lock() )
    locked->SetReady( lobby::EGameMemberReadiness::ReadyForAnything ); //TODO: lobby::EGameMemberReadiness::ReadyForAnything
}

void SelectHeroScreenLogic::LeaveLobby()
{ 
  if ( StrongMT<Game::IGameContextUiInterface> locked = screen->GameCtx().Lock() ) {
    isPlayerReady = false;
    needUpdatePlayerReadyness = false;
    g_needNotifyLobbyClients = false;

    NScreenCommands::PushCommand( NScreenCommands::CreatePopScreenCommand( screen.Get() ) );
    locked->SetReady( lobby::EGameMemberReadiness::NotReady );
    locked->ConnectToCluster( g_devLogin, "123456" );
  }
}

void SelectHeroScreenLogic::ChangeTeam( int team )
{
  if ( StrongMT<Game::IGameContextUiInterface> locked = screen->GameCtx().Lock() )
    locked->ChangeCustomGameSettings( team == 0 ? lobby::ETeam::Team1 : lobby::ETeam::Team2, lobby::ETeam::None, string() );
}

void SelectHeroScreenLogic::ChangeFaction( int faction )
{
  if ( StrongMT<Game::IGameContextUiInterface> locked = screen->GameCtx().Lock() )
    locked->ChangeCustomGameSettings( lobby::ETeam::None, faction == 0 ? lobby::ETeam::Team1 : lobby::ETeam::Team2, string() );
}

void SelectHeroScreenLogic::SetDeveloperParty(int party)
{
  if ( StrongMT<Game::IGameContextUiInterface> locked = screen->GameCtx().Lock() )
    locked->SetDeveloperParty(party);
}

void SelectHeroScreenLogic::DebugDisplayPlayers ( const wstring & status )
{
  
  if (isPlayerReady && needUpdatePlayerReadyness) {
    if ( StrongMT<Game::IGameContextUiInterface> locked = screen->GameCtx().Lock() ) {
      locked->SetReady( lobby::EGameMemberReadiness::ReadyForAnything );
      needUpdatePlayerReadyness = false;
    }
  }
  if (isPlayerReady && g_needNotifyLobbyClients) { // Update ready status for everyone
    if ( StrongMT<Game::IGameContextUiInterface> locked = screen->GameCtx().Lock() ) {
      locked->SetReady( lobby::EGameMemberReadiness::NotReady );
      needUpdatePlayerReadyness = true;
      g_needNotifyLobbyClients = false;
    }
  }
  
  UI::ImageLabel * pDesc = UI::GetChildChecked < UI::ImageLabel > ( pBaseWindow, "DebugPlayers", true );
  if ( pDesc )
    pDesc->SetCaptionTextW ( status.c_str() );
}

}//end of namespace


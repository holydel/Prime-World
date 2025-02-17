#include "stdafx.h"
#include "NewLobbyClientPW.h"
#include "PF_GameLogic/MapCollection.h"
#include "PF_GameLogic/StringExecutor.h"
#include "PF_GameLogic/AdventureScreen.h"
#include "Client/ScreenCommands.h"

#include "SelectGameModeScreen.h"
#include "SelectHeroScreen.h"
#include "FastReconnectCtxPW.h"


bool g_needNotifyLobbyClients = false;
string g_selectedHeroes[10];

namespace lobby
{

ClientPW::ClientPW( Transport::TClientId _clientId, bool _socialMode, Game::IGameContextUiInterface * _gameCtx, FastReconnectCtxPW * _fastReconnectCtx ) :
ClientBase( _clientId, _socialMode ),
gameCtx( _gameCtx ),
fastReconnectCtxPw( _fastReconnectCtx ),
lastRememberedPlayers ( -1 ) 
{
	fillHeroNeme();
}



void ClientPW::OnStatusChange( EClientStatus::Enum newStatus )
{
  ClientBase::OnStatusChange( newStatus );

  if ( newStatus == EClientStatus::Connected )
  {
    if ( fastReconnectCtxPw )
      mapCollection = fastReconnectCtxPw->Maps();
    else
    {
      StrongMT<NWorld::PWMapCollection> maps = new NWorld::PWMapCollection;
      maps->ScanForMaps();
      mapCollection = maps;
    }

    if ( !InSocialMode() )
      GotoFirstLobbyScreen();
  }
  else if ( newStatus == EClientStatus::InCustomLobby )
  {
    RemoveFirstLobbyScreen();

    if ( !fastReconnectCtxPw ) {
      heroScreen = new NGameX::SelectHeroScreen( gameCtx.Lock() );
      NScreenCommands::PushCommand( NScreenCommands::CreatePushScreenCommand( heroScreen ) );
    }
  }
  else if ( newStatus == EClientStatus::InGameSession )
  {
    RemoveFirstLobbyScreen(); // Needed here for custom game reconnect, where we skip InCustomLobby

    if ( heroScreen )
      NScreenCommands::PushCommand( NScreenCommands::CreatePopScreenCommand( heroScreen ) );
  }
}



void ClientPW::OnLobbyDataChange()
{
  ClientBase::OnLobbyDataChange();

  if ( Status() == EClientStatus::InCustomLobby )
  {
    set<int> empty;
    UpdateCustomLobbyPlayers( empty );
  }
}


void ClientPW::CreateGame( const char * mapId, int maxPlayers, int autostartPlayers /*= -1*/ )
{
  StrongMT<NWorld::IMapLoader> mapLoader = mapCollection->CreateMapLoader( mapId );
  NI_DATA_VERIFY( mapLoader, NStr::StrFmt( "Could not load map '%s'", mapId ), return );

  ClientBase::CreateGame( mapId, maxPlayers, mapLoader->GetMaxPlayersPerTeam(), autostartPlayers );
  //int gameId = gameParams.ID;
  //string tmp(gameId);

  //OutputDebugStringA(tmp.c_str()); 
}



void ClientPW::GotoFirstLobbyScreen()
{
  if ( fastReconnectCtxPw )
    return;

  //Lobby server requested Player interface, we ready to show GUI
  lobbyScreen = new NGameX::SelectGameModeScreen( gameCtx.Lock() );

  NScreenCommands::PushCommand( NScreenCommands::CreatePushScreenCommand( lobbyScreen ) );
}



void ClientPW::RemoveFirstLobbyScreen()
{
  if ( lobbyScreen )
    NScreenCommands::PushCommand( NScreenCommands::CreatePopScreenCommand( lobbyScreen ) );
}

void ClientPW::UpdateCustomLobbyPlayers( const set<int> & hilitePlayers )
{
  if ( !heroScreen )
    return;
  vector<wstring> lines;
  vector<int> linesIds;

  for (int i = 0; i < 10; ++i) {
    g_selectedHeroes[i].clear();
  }

  int readyPlayers = 0;
  int heroCounter = 0;

  for( int i = 0; i < 2; ++i )
  {
    ETeam::Enum team = i ? ETeam::Team2 : ETeam::Team1;

    lines.push_back( i ? L"������� ����������:" : L"������� ������:" );
    linesIds.push_back( -1 );
    for ( int j = 0; j < GameLineup().size(); ++j )
    {
      const SGameMember & memb = GameLineup()[j];
      if( memb.context.team != team )
        continue;
      
      const bool ready = ( ReadyPlayers().find( memb.user.userId ) != ReadyPlayers().end() );
      readyPlayers += ready ? 1 : 0;
      g_selectedHeroes[heroCounter] = memb.context.hero.c_str();

      string newHeroName;

	    if(HeroNameLobby.count(g_selectedHeroes[heroCounter])){
		    newHeroName = (memb.context.original_team == lobby::ETeam::Team2) ? HeroNameLobby[g_selectedHeroes[heroCounter]].HeroNameB : HeroNameLobby[g_selectedHeroes[heroCounter]].HeroNameA;
	    }
      wstring line = NStr::StrFmtW( L"<space:2>%s (%d)  %s %s, %s",
        memb.user.nickname.c_str(),
        memb.user.userId,
        NStr::ToUnicode( newHeroName ).c_str(),
        //memb.context.original_team == lobby::ETeam::Team2 ? L"(��������)" : L"(����)",
        memb.context.original_team == lobby::ETeam::Team2 ? L"" : L"", // disabled for some time since skins are random
        ready ? L"<style:green>ready</style>" : L"<style:money>not ready</style>" );

      lines.push_back( line );
      linesIds.push_back( memb.user.userId );
      heroCounter++;
    }
  }

  int totalPlayers = lines.size();
  if (lastRememberedPlayers != totalPlayers) {
    // Notify all members to be ready only if new not every player is ready
    // In other case all players will be ready to play, but someone left the game
    if (totalPlayers == readyPlayers) {
      g_needNotifyLobbyClients = true;
    }
  }
  // If someone just connected - notify
  if (lastRememberedPlayers < totalPlayers) {
    g_needNotifyLobbyClients = true;
  }
  lastRememberedPlayers = totalPlayers;

  heroScreen->UpdatePlayers( lines, linesIds, hilitePlayers );
}

void ClientPW::Award( const vector<roll::SAwardInfo> & _awards )
{
  if (advScreen)
    advScreen->AwardUser(_awards);
}

void ClientPW::CleanUIScreens()
{
  RemoveFirstLobbyScreen();
}

ClientPW::HeroName::HeroName(string nameA, string nameB)
{
	ClientPW::HeroName::HeroNameA = nameA;
	ClientPW::HeroName::HeroNameB = nameB;
}

ClientPW::HeroName::HeroName(){}

void ClientPW::fillHeroNeme()
{
	HeroNameLobby["healer"] = ClientPW::HeroName("������������", "�����");
	HeroNameLobby["rockman"] = ClientPW::HeroName("�������-����", "�����");
	HeroNameLobby["mowgly"] = ClientPW::HeroName("�����", "��������� � �������");
	HeroNameLobby["ratcatcher"] = ClientPW::HeroName("��������", "���������� ����");
	HeroNameLobby["thundergod"] = ClientPW::HeroName("������������", "�����������");
	HeroNameLobby["manawyrm"] = ClientPW::HeroName("��������", "��������");
	HeroNameLobby["witchdoctor"] = ClientPW::HeroName("�����", "�������");
	HeroNameLobby["faceless"] = ClientPW::HeroName("��������", "����� �����");
	HeroNameLobby["highlander"] = ClientPW::HeroName("�����", "�����������");
	HeroNameLobby["night"] = ClientPW::HeroName("������ ����", "������ �������");
	HeroNameLobby["firefox"] = ClientPW::HeroName("�������� ����", "����� �����");
	HeroNameLobby["unicorn"] = ClientPW::HeroName("����", "�����");
	HeroNameLobby["frogenglut"] = ClientPW::HeroName("����� ��������", "�������� ����");
	HeroNameLobby["prince"] = ClientPW::HeroName("�������", "����� �����");
	HeroNameLobby["warlord"] = ClientPW::HeroName("�������", "������������");
	HeroNameLobby["hunter"] = ClientPW::HeroName("�������", "�������");
	HeroNameLobby["mage"] = ClientPW::HeroName("�����������", "�������");
	HeroNameLobby["naga"] = ClientPW::HeroName("������ �������", "���������� �����");
	HeroNameLobby["werewolf"] = ClientPW::HeroName("����", "������");
	HeroNameLobby["invisible"] = ClientPW::HeroName("����", "���������");
	HeroNameLobby["assassin"] = ClientPW::HeroName("����������", "�������");
	HeroNameLobby["ghostlord"] = ClientPW::HeroName("���� ���", "�������");
	HeroNameLobby["marine"] = ClientPW::HeroName("������", "�������");
	HeroNameLobby["snowqueen"] = ClientPW::HeroName("����", "�����");
	HeroNameLobby["archeress"] = ClientPW::HeroName("�������", "��������");
	HeroNameLobby["inventor"] = ClientPW::HeroName("������������", "������������");
	HeroNameLobby["bard"] = ClientPW::HeroName("����", "����");
	HeroNameLobby["fairy"] = ClientPW::HeroName("�����", "�������� ���");
	HeroNameLobby["artist"] = ClientPW::HeroName("���������", "���������");
	HeroNameLobby["witcher"] = ClientPW::HeroName("�������", "�������");
	HeroNameLobby["demonolog"] = ClientPW::HeroName("���������", "���������");
	HeroNameLobby["alchemist"] = ClientPW::HeroName("��������", "��������");
	HeroNameLobby["vampire"] = ClientPW::HeroName("������", "�����");
	HeroNameLobby["witch"] = ClientPW::HeroName("������", "�����");
	HeroNameLobby["crusader_A"] = ClientPW::HeroName("��'��", "��'��");
	HeroNameLobby["crusader_B"] = ClientPW::HeroName("��'��", "��'��");
	HeroNameLobby["monster"] = ClientPW::HeroName("��������", "������");
	HeroNameLobby["angel"] = ClientPW::HeroName("����������", "�������������");
	HeroNameLobby["freeze"] = ClientPW::HeroName("����", "����");
	HeroNameLobby["gunslinger"] = ClientPW::HeroName("���������", "���������");
	HeroNameLobby["reaper"] = ClientPW::HeroName("��'����", "��'����");
	HeroNameLobby["fluffy"] = ClientPW::HeroName("����", "����");
	HeroNameLobby["rifleman"] = ClientPW::HeroName("������", "������");
	HeroNameLobby["magicgirl"] = ClientPW::HeroName("����", "����");
	HeroNameLobby["pinkgirl"] = ClientPW::HeroName("���������", "���������");
	HeroNameLobby["ironknight"] = ClientPW::HeroName("�������", "�������");
	HeroNameLobby["fallenangel"] = ClientPW::HeroName("������", "������");
	HeroNameLobby["bladedancer"] = ClientPW::HeroName("����", "����");
	HeroNameLobby["ent"] = ClientPW::HeroName("�������", "�������");
	HeroNameLobby["plaguedoctor"] = ClientPW::HeroName("������ ������", "������ ������");
	HeroNameLobby["katana"] = ClientPW::HeroName("������", "������");
	HeroNameLobby["plane"] = ClientPW::HeroName("�������", "�������");
	HeroNameLobby["zealot"] = ClientPW::HeroName("�������", "�������");
	HeroNameLobby["wraithking"] = ClientPW::HeroName("������ �������", "������ �������");
	HeroNameLobby["dryad"] = ClientPW::HeroName("������", "������");
	HeroNameLobby["stalker"] = ClientPW::HeroName("��������", "��������");
	HeroNameLobby["gunner"] = ClientPW::HeroName("�������", "�������");
	HeroNameLobby["chronicle"] = ClientPW::HeroName("�������", "�������");
	HeroNameLobby["brewer"] = ClientPW::HeroName("�������", "�������");
	HeroNameLobby["shadow"] = ClientPW::HeroName("����", "����");
	HeroNameLobby["wendigo"] = ClientPW::HeroName("�������", "�������");
	HeroNameLobby["trickster"] = ClientPW::HeroName("��������", "��������");
	HeroNameLobby["banshee"] = ClientPW::HeroName("�����", "�����");
	HeroNameLobby["shaman"] = ClientPW::HeroName("�����", "�����");
	HeroNameLobby["bomber"] = ClientPW::HeroName("����������", "����������");
}

} //namespace lobby

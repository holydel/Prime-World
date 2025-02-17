#include "stdafx.h"

#include "BattleServer.h"
#include "Protocol.h"
#include "LBattleServer.auto.h"

namespace Battle
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BattleServer::BattleServer( Transport::ITransportSystem* _transport, const Transport::TServiceId& serviceId )
{
  gateKeeper = new rpc::GateKeeper( _transport, serviceId, Transport::autoAssignClientId, this );
  gateKeeper->GetGate().RegisterObject<BattleServer>( this, Protocol::BATTLE_SERVER_RPC_NAME );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BattleServer::~BattleServer()
{

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void BattleServer::OnNewNode( Transport::IChannel* channel,  rpc::Node* node )
{

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void BattleServer::OnChannelClosed( Transport::IChannel* channel,  rpc::Node* node )
{

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void BattleServer::OnCorruptData( Transport::IChannel* channel, rpc::Node* node )
{

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void BattleServer::Step( float dt, NHPTimer::STime serverTime )
{
  gateKeeper->Poll();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void BattleServer::TestFunction( int _a )
{

}

} // namespace Battle

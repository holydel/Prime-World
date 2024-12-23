#include "stdafx.h"
#include "StateCreatingGame.h"
#include "../Net/NetDriver.h"
#include "CoreFSM.h"
#include "NetMessagesProcessor.h"
#include "StateWaitingRoomHost.h"
#include "MessagesGeneral.h"

namespace NCore
{
	CreatingGameState::CreatingGameState( CoreFSM* context, NNet::IDriver* netDriver )
		: CoreFSMState( context )
		, NetworkState( netDriver )
	{
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void CreatingGameState::SubscribeUpdates()
	{
		CoreFSMState::SubscribeUpdates();
		pContext->RegisterMessageBuilder( GetTypeId(), &NCore::Waiting::NewWaiting );
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void CreatingGameState::ProcessNetMessage( int clientId, CNetPacket* packet )
	{
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void CreatingGameState::ProcessClientAdd( int clientId )
	{
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void CreatingGameState::ProcessClientRemove( int clientId )
	{
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void CreatingGameState::ProcessKicked()
	{
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	IBaseFSMState* CreatingGameState::Step(float)
	{
		NI_ALWAYS_ASSERT( "Not implemented yet!" );

		// process net packets
		ProcessMessages();

		// create host...		
		MapStartInfo startInfo;
    return new WaitingRoomHostState( pContext, GetDriver(), startInfo, -1, -1, MapStartInfo::BALANCE_ALL, -1 );
	}
}

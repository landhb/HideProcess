#include "ndis_sniff.h"


#pragma alloc_text(INIT, NdisHookProtocolFind)
#pragma alloc_text(INIT, NdisHookProtocolEnumOpened)
#pragma alloc_text(INIT, NdisHookOpenGetMiniport)
#pragma alloc_text(INIT, NdisHookAllocJump)
#pragma alloc_text(INIT, NdisHookSet)

// field offsets for NDIS structures
int NDIS_PROTOCOL_BLOCK_Name = -1,
NDIS_PROTOCOL_BLOCK_OpenQueue = -1,
NDIS_PROTOCOL_BLOCK_NextProtocol = -1,
NDIS_OPEN_BLOCK_ProtocolNextOpen = -1,
NDIS_OPEN_BLOCK_MiniportHandle = -1,
NDIS_MINIPORT_BLOCK_InterruptEx = -1,
NDIS_MINIPORT_BLOCK_IndicateNetBufferListsHandler = -1,
NDIS_INTERRUPT_BLOCK_MiniportDpc = -1;


VOID NdisprotReceiveNetBufferLists(
	IN NDIS_HANDLE                  ProtocolBindingContext,
	IN PNET_BUFFER_LIST             pNetBufferLists,
	IN NDIS_PORT_NUMBER             PortNumber,
	IN ULONG                        NumberOfNetBufferLists,
	IN ULONG                        ReceiveFlags
) {
	/*
	Routine Description:
		Protocol entry point called by NDIS if the driver below
		uses NDIS 6 net buffer list indications.
		If the miniport allows us to hold on to this net buffer list, we
		use it as is, otherwise we make a copy.
	Arguments:
		ProtocolBindingContext - pointer to open context
		pNetBufferLists - a list of the Net Buffer lists being indicated up.
		PortNumber - Port on which the Net Bufer list was received
		NumberOfNetBufferLists - the number of NetBufferLists in this indication
		ReceiveFlags - indicates whether the NetBufferLists can be pended in the protocol driver.

	*/

	UNREFERENCED_PARAMETER(ProtocolBindingContext);
	UNREFERENCED_PARAMETER(pNetBufferLists);
	UNREFERENCED_PARAMETER(PortNumber);
	UNREFERENCED_PARAMETER(NumberOfNetBufferLists);
	UNREFERENCED_PARAMETER(ReceiveFlags);
	return;

} 

#include "ndis_sniff.h"


//--------------------------------------------------------------------------------------
NDIS_STATUS
NdisprotBindAdapter(
	IN NDIS_HANDLE                  ProtocolDriverContext,
	IN NDIS_HANDLE                  BindContext,
	IN PNDIS_BIND_PARAMETERS        BindParameters
)
{
    /*
        Routine Description:
			Protocol Bind Handler entry point called when NDIS wants us
			to bind to an adapter. We go ahead and set up a binding.
			An OPEN_CONTEXT structure is allocated to keep state about
			this binding.
    */

	UNREFERENCED_PARAMETER(BindContext);
	UNREFERENCED_PARAMETER(BindParameters);
	UNREFERENCED_PARAMETER(ProtocolDriverContext);

	return NDIS_STATUS_SUCCESS;
}
//--------------------------------------------------------------------------------------
VOID
NdisprotOpenAdapterComplete(
	IN NDIS_HANDLE                  ProtocolBindingContext,
	IN NDIS_STATUS                  Status
)
{
    /*
        This function is a required driver function that completes processing of a binding 
        operation for which NdisOpenAdapter returned NDIS_STATUS_PENDING.

		Arguments:
			ProtocolBindingContext - pointer to open context structure
			Status - status of the open
		
    */
	UNREFERENCED_PARAMETER(ProtocolBindingContext);
	UNREFERENCED_PARAMETER(Status);
	
	return;
}
//--------------------------------------------------------------------------------------
NDIS_STATUS NdisprotUnbindAdapter(
	IN NDIS_HANDLE                  UnbindContext,
	IN NDIS_HANDLE                  ProtocolBindingContext
)
{
    /*
        Routine Description:
			NDIS calls this when it wants us to close the binding to an adapter.
		Arguments:
			ProtocolBindingContext - pointer to open context structure
			UnbindContext - to use in NdisCompleteUnbindAdapter if we return pending
		Return Value:
			pending or success
    */
	UNREFERENCED_PARAMETER(ProtocolBindingContext);
	UNREFERENCED_PARAMETER(UnbindContext);
	
	return NDIS_STATUS_SUCCESS;

}


VOID NdisprotCloseAdapterComplete(
	IN NDIS_HANDLE                  ProtocolBindingContext
)

{
    /*
        This function is a required driver function that completes processing for an unbinding 
        operation for which NdisCloseAdapter returned NDIS_STATUS_PENDING.

		Routine Description:
			Called by NDIS to complete a pended call to NdisCloseAdapter.
			We wake up the thread waiting for this completion.
		Arguments:
			ProtocolBindingContext - pointer to open context structure

    */
	UNREFERENCED_PARAMETER(ProtocolBindingContext);

	return;
}


//--------------------------------------------------------------------------------------
VOID NdisprotStatus(
	IN NDIS_HANDLE                  ProtocolBindingContext,
	IN PNDIS_STATUS_INDICATION      StatusIndication
)
{
    /*
        This function is a required driver function that handles status-change notifications 
        raised by an underlying connectionless network adapter driver or by NDIS.

		Routine Description:
		Protocol entry point called by NDIS to indicate a change
		in status at the miniport.
	
		Arguments:
		ProtocolBindingContext - pointer to open context
		StatusIndication - pointer to NDIS_STATUS_INDICATION
    */
	UNREFERENCED_PARAMETER(ProtocolBindingContext);
	UNREFERENCED_PARAMETER(StatusIndication);

	return;
}


VOID NdisprotSendComplete(
	IN NDIS_HANDLE                  ProtocolBindingContext,
	IN PNET_BUFFER_LIST             pNetBufferList,
	IN ULONG                        SendCompleteFlags
)
{
    /*
		Routine Description:
			NDIS entry point called to signify completion of a packet send.
			We pick up and complete the Write IRP corresponding to this packet.
		Arguments:
			ProtocolBindingContext - pointer to open context
			pNetBufferList - NetBufferList that completed send
			SendCompleteFlags - Specifies if the caller is at DISPATCH level
    */
	UNREFERENCED_PARAMETER(ProtocolBindingContext);
	UNREFERENCED_PARAMETER(pNetBufferList);
	UNREFERENCED_PARAMETER(SendCompleteFlags);


	return;
}       



//--------------------------------------------------------------------------------------

NDIS_STATUS NdisprotPnPEventHandler(
	IN NDIS_HANDLE                  ProtocolBindingContext,
	IN PNET_PNP_EVENT_NOTIFICATION  pNetPnPEventNotification
)
{

	/*
	
	Routine Description:
		Called by NDIS to notify us of a PNP event. The most significant
		one for us is power state change.
	Arguments:
		ProtocolBindingContext - pointer to open context structure
								this is NULL for global reconfig events.
		pNetPnPEventNotification - pointer to the PNP event notification

	*/
	UNREFERENCED_PARAMETER(ProtocolBindingContext);
	UNREFERENCED_PARAMETER(pNetPnPEventNotification);
    return NDIS_STATUS_SUCCESS;
}


VOID NdisprotRequestComplete(
	IN NDIS_HANDLE                  ProtocolBindingContext,
	IN PNDIS_OID_REQUEST            pNdisRequest,
	IN NDIS_STATUS                  Status
)

{
	/*
	Routine Description:
		NDIS entry point indicating completion of a pended NDIS_REQUEST.
	Arguments:
		ProtocolBindingContext - pointer to open context
		pNdisRequest - pointer to NDIS request
		Status - status of reset completion
	*/

	UNREFERENCED_PARAMETER(ProtocolBindingContext);
	UNREFERENCED_PARAMETER(pNdisRequest);
	UNREFERENCED_PARAMETER(Status);
	return;
}

//--------------------------------------------------------------------------------------
NTSTATUS BogusProtocolRegister()
{

	NDIS_PROTOCOL_DRIVER_CHARACTERISTICS   protocolChar;
	NTSTATUS status = STATUS_SUCCESS;
	NDIS_STRING protoName = NDIS_STRING_CONST("NDISPROT");

    if (&Globals.NdisProtocolHandle)
    {
		// Protocol is already registered
        return STATUS_UNSUCCESSFUL;
    }

	//
	// Initialize the protocol characterstic structure
	// Taken directly from MSDN example, thank you Microsoft
	//

	NdisZeroMemory(&protocolChar, sizeof(NDIS_PROTOCOL_DRIVER_CHARACTERISTICS));


	protocolChar.Header.Type = NDIS_OBJECT_TYPE_PROTOCOL_DRIVER_CHARACTERISTICS,
	protocolChar.Header.Size = sizeof(NDIS_PROTOCOL_DRIVER_CHARACTERISTICS);
	protocolChar.Header.Revision = NDIS_PROTOCOL_DRIVER_CHARACTERISTICS_REVISION_1;

	protocolChar.MajorNdisVersion = 6;
	protocolChar.MinorNdisVersion = 0;
	protocolChar.Name = protoName;
	protocolChar.SetOptionsHandler = NULL;
	protocolChar.OpenAdapterCompleteHandlerEx = NdisprotOpenAdapterComplete;
	protocolChar.CloseAdapterCompleteHandlerEx = NdisprotCloseAdapterComplete;
	protocolChar.SendNetBufferListsCompleteHandler = NdisprotSendComplete;
	protocolChar.OidRequestCompleteHandler = NdisprotRequestComplete;
	protocolChar.StatusHandlerEx = NdisprotStatus;
	protocolChar.UninstallHandler = NULL;
	protocolChar.ReceiveNetBufferListsHandler = NdisprotReceiveNetBufferLists;
	protocolChar.NetPnPEventHandler = NdisprotPnPEventHandler;
	protocolChar.BindAdapterHandlerEx = NdisprotBindAdapter;
	protocolChar.UnbindAdapterHandlerEx = NdisprotUnbindAdapter;

	//
	// Register as a protocol driver
	//

	status = NdisRegisterProtocolDriver(NULL,           // driver context
		&protocolChar,
		&Globals.NdisProtocolHandle);

	if (status != NDIS_STATUS_SUCCESS)
	{
		// Failed to register protocol with NDIS
		return STATUS_UNSUCCESSFUL;
	}

    return status;    
}
//--------------------------------------------------------------------------------------
void BogusProtocolUnregister()
{
  
	NDIS_HANDLE     ProtocolHandle;

	// Grab the handle to our protocol and Deregister it

	if (Globals.NdisProtocolHandle != NULL)
	{
		ProtocolHandle = Globals.NdisProtocolHandle;
		Globals.NdisProtocolHandle = NULL;

		NdisDeregisterProtocolDriver(ProtocolHandle);

	}
}
//--------------------------------------------------------------------------------------
// EoF

/*++

Copyright (c) 2000  Microsoft Corporation

Module Name:

ndisprot.h

Abstract:

Data structures, defines and function prototypes for NDISPROT.

Environment:

Kernel mode only.

--*/

#pragma once
#pragma warning(disable: 4201)

// NDIS version: 6.0
// Microsoft required pre-processor definition
#define NDIS60 1

#define WIN9X_COMPAT_SPINLOCK

#include "ndis.h"
#include "ntddk.h"
#include <wdf.h>
#include <wmistr.h>
#include <wdmsec.h>
#include <wdmguid.h>
#include "ndis_debug.h"



#define NT_DEVICE_NAME          L"\\Device\\Ndisprot"
#define DOS_DEVICE_NAME         L"\\Global??\\Ndisprot"


//
//  Abstract types
//
typedef NDIS_EVENT              NPROT_EVENT, *PNPROT_EVENT;

#define MAX_MULTICAST_ADDRESS   0x20

#define NPROT_MAC_ADDR_LEN            6

//-----------------------------------------------------------------------------
// 4127 -- Conditional Expression is Constant warning
//-----------------------------------------------------------------------------
#define WHILE(constant) \
__pragma(warning(disable: 4127)) while(constant); __pragma(warning(default: 4127))

//
// Message verbosity: lower values indicate higher urgency
//
#define DL_EXTRA_LOUD       20
#define DL_VERY_LOUD        10
#define DL_LOUD             8
#define DL_INFO             6
#define DL_WARN             4
#define DL_ERROR            2
#define DL_FATAL            0


typedef struct _NPROT_LOCK
{
	ULONG                   Signature;
	ULONG                   IsAcquired;
	PKTHREAD                OwnerThread;
	ULONG                   TouchedByFileNumber;
	ULONG                   TouchedInLineNumber;
	NDIS_SPIN_LOCK          NdisLock;
} NPROT_LOCK, *PNPROT_LOCK;

typedef enum _NDISPROT_OPEN_STATE {
	NdisprotInitializing,
	NdisprotRunning,
	NdisprotPausing,
	NdisprotPaused,
	NdisprotRestarting,
	NdisprotClosing
} NDISPROT_OPEN_STATE;
//
//  The Open Context represents an open of our device object.
//  We allocate this on processing a BindAdapter from NDIS,
//  and free it when all references (see below) to it are gone.
//
//  Binding/unbinding to an NDIS device:
//
//  On processing a BindAdapter call from NDIS, we set up a binding
//  to the specified NDIS device (miniport). This binding is
//  torn down when NDIS asks us to Unbind by calling
//  our UnbindAdapter handler.
//
//  Receiving data:
//
//  While an NDIS binding exists, read IRPs are queued on this
//  structure, to be processed when packets are received.
//  If data arrives in the absense of a pended read IRP, we
//  queue it, to the extent of one packet, i.e. we save the
//  contents of the latest packet received. We fail read IRPs
//  received when no NDIS binding exists (or is in the process
//  of being torn down).
//
//  Sending data:
//
//  Write IRPs are used to send data. Each write IRP maps to
//  a single NDIS packet. Packet send-completion is mapped to
//  write IRP completion. We use NDIS 5.1 CancelSend to support
//  write IRP cancellation. Write IRPs that arrive when we don't
//  have an active NDIS binding are failed.
//
//  Reference count:
//
//  The following are long-lived references:
//  OPEN_DEVICE ioctl (goes away on processing a Close IRP)
//  Pended read IRPs
//  Queued received packets
//  Uncompleted write IRPs (outstanding sends)
//  Existence of NDIS binding
//
typedef struct _NDISPROT_OPEN_CONTEXT
{
	LIST_ENTRY              Link;           // Link into global list
	ULONG                   Flags;          // State information
	ULONG                   RefCount;
	NPROT_LOCK               Lock;

	WDFFILEOBJECT            pFileObject;    // Set on OPEN_DEVICE

	NDIS_HANDLE             BindingHandle;
	NDIS_HANDLE             SendNetBufferListPool;
	// let every net buffer list contain one net buffer(don't know how many net buffers can be include in one list.
	NDIS_HANDLE             RecvNetBufferListPool;

	ULONG                   MacOptions;
	ULONG                   MaxFrameSize;
	ULONG                   DataBackFillSize;
	ULONG                   ContextBackFillSize;

	ULONG                   PendedSendCount;

	WDFQUEUE                ReadQueue;
	ULONG                   PendedReadCount;
	LIST_ENTRY              RecvNetBufListQueue;
	ULONG                   RecvNetBufListCount;

	NET_DEVICE_POWER_STATE  PowerState;
	NDIS_EVENT              PoweredUpEvent; // signalled iff PowerState is D0
	NDIS_STRING             DeviceName;     // used in NdisOpenAdapter
	NDIS_STRING             DeviceDescr;    // friendly name

	NDIS_STATUS             BindStatus;     // for Open/CloseAdapter
	NPROT_EVENT             BindEvent;      // for Open/CloseAdapter

	ULONG                   oc_sig;         // Signature for sanity
	NDISPROT_OPEN_STATE     State;
	PNPROT_EVENT            ClosingEvent;
	UCHAR                   CurrentAddress[NPROT_MAC_ADDR_LEN];
	UCHAR                   MCastAddress[MAX_MULTICAST_ADDRESS][NPROT_MAC_ADDR_LEN];

	WDFQUEUE             StatusIndicationQueue;
} NDISPROT_OPEN_CONTEXT, *PNDISPROT_OPEN_CONTEXT;

typedef struct _FILEO_BJECT_CONTEXT {

	PNDISPROT_OPEN_CONTEXT OpenContext;

} FILE_OBJECT_CONTEXT, *PFILE_OBJECT_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(FILE_OBJECT_CONTEXT, GetFileObjectContext)

typedef struct _REQUEST_CONTEXT {

	PNET_BUFFER_LIST NetBufferList;    // used if we had to partial-map
	ULONG Length;

} REQUEST_CONTEXT, *PREQUEST_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(REQUEST_CONTEXT, GetRequestContext)

#define oc_signature        'OiuN'

//
//  Definitions for Flags above.
//
#define NPROTO_BIND_IDLE             0x00000000
#define NPROTO_BIND_OPENING          0x00000001
#define NPROTO_BIND_FAILED           0x00000002
#define NPROTO_BIND_ACTIVE           0x00000004
#define NPROTO_BIND_CLOSING          0x00000008
#define NPROTO_BIND_FLAGS            0x0000000F  // State of the binding

#define NPROTO_OPEN_IDLE             0x00000000
#define NPROTO_OPEN_ACTIVE           0x00000010
#define NPROTO_OPEN_FLAGS            0x000000F0  // State of the I/O open

#define NPROTO_RESET_IN_PROGRESS     0x00000100
#define NPROTO_NOT_RESETTING         0x00000000
#define NPROTO_RESET_FLAGS           0x00000100

#define NPROTO_MEDIA_CONNECTED       0x00000000
#define NPROTO_MEDIA_DISCONNECTED    0x00000200
#define NPROTO_MEDIA_FLAGS           0x00000200

#define NPROTO_READ_SERVICING        0x00100000  // Is the read service
// routine running?
#define NPROTO_READ_FLAGS            0x00100000

#define NPROTO_UNBIND_RECEIVED       0x10000000  // Seen NDIS Unbind?
#define NPROTO_UNBIND_FLAGS          0x10000000


#define NPROT_ALLOCATED_NBL          0x10000000
#define NPROT_NBL_RETREAT_RECV_RSVD  0x20000000

//
//  Globals:
//
typedef struct _NDISPROT_GLOBALS
{
	PDRIVER_OBJECT          DriverObject;
	WDFDEVICE               ControlDevice;
	NDIS_HANDLE             NdisProtocolHandle;
	USHORT                  EthType;            // frame type we are interested in
	UCHAR                   PartialCancelId;    // for cancelling sends
	ULONG                   LocalCancelId;
	LIST_ENTRY              OpenList;           // of OPEN_CONTEXT structures
	NPROT_LOCK              GlobalLock;         // to protect the above
	NPROT_EVENT             BindsComplete;      // have we seen NetEventBindsComplete?
} NDISPROT_GLOBALS, *PNDISPROT_GLOBALS;


//
//  The following are arranged in the way a little-endian processor
//  would read 2 bytes off the wire.
//
#define NPROT_ETH_TYPE               0x8e88
#define NPROT_8021P_TAG_TYPE         0x0081

//
//  NDIS Request context structure
//
typedef struct _NDISPROT_REQUEST
{
	NDIS_OID_REQUEST         Request;
	NPROT_EVENT              ReqEvent;
	ULONG                    Status;

} NDISPROT_REQUEST, *PNDISPROT_REQUEST;


#define NPROTO_PACKET_FILTER  (NDIS_PACKET_TYPE_DIRECTED|    \
                              NDIS_PACKET_TYPE_MULTICAST|   \
                              NDIS_PACKET_TYPE_BROADCAST)

//
//  Send packet pool bounds
//
/*
#define MIN_SEND_PACKET_POOL_SIZE    20
*/
#define MAX_SEND_PACKET_POOL_SIZE    400


//
//  ProtocolReserved in sent packets. We save a pointer to the IRP
//  that generated the send.
//
//  The RefCount is used to determine when to free the packet back
//  to its pool. It is used to synchronize between a thread completing
//  a send and a thread attempting to cancel a send.
//
typedef struct _NPROT_SEND_NETBUFLIST_RSVD
{
	WDFREQUEST              Request;
	ULONG                   RefCount;

} NPROT_SEND_NETBUFLIST_RSVD, *PNPROT_SEND_NETBUFLIST_RSVD;
//
//  Receive packet pool bounds
//
#define MIN_RECV_PACKET_POOL_SIZE    4
#define MAX_RECV_PACKET_POOL_SIZE    20

//
//  Max receive packets we allow to be queued up
//
#define MAX_RECV_QUEUE_SIZE          4

//
//  ProtocolReserved in received packets: we link these
//  packets up in a queue waiting for Read IRPs.
//
typedef struct _NPROT_RECV_NBL_RSVD
{
	LIST_ENTRY              Link;
	PNET_BUFFER_LIST        pNetBufferList;    // used if we had to partial-map

} NPROT_RECV_NBL_RSVD, *PNPROT_RECV_NBL_RSVD;


#include <pshpack1.h>

typedef struct _NDISPROT_ETH_HEADER
{
	UCHAR       DstAddr[NPROT_MAC_ADDR_LEN];
	UCHAR       SrcAddr[NPROT_MAC_ADDR_LEN];
	USHORT      EthType;

} NDISPROT_ETH_HEADER;

typedef struct _NDISPROT_ETH_HEADER UNALIGNED * PNDISPROT_ETH_HEADER;

#include <poppack.h>


extern NDISPROT_GLOBALS      Globals;


#define NPROT_ALLOC_TAG      'oiuN'


//
//  Prototypes.
//



PROTOCOL_BIND_ADAPTER_EX NdisprotBindAdapter;

PROTOCOL_OPEN_ADAPTER_COMPLETE_EX NdisprotOpenAdapterComplete;

PROTOCOL_UNBIND_ADAPTER_EX NdisprotUnbindAdapter;

PROTOCOL_CLOSE_ADAPTER_COMPLETE_EX NdisprotCloseAdapterComplete;


PROTOCOL_NET_PNP_EVENT NdisprotPnPEventHandler;

VOID
NdisprotProtocolUnloadHandler(
	VOID
);


PROTOCOL_OID_REQUEST_COMPLETE NdisprotRequestComplete;


PROTOCOL_STATUS_EX NdisprotStatus;

PROTOCOL_RECEIVE_NET_BUFFER_LISTS NdisprotReceiveNetBufferLists;


PROTOCOL_SEND_NET_BUFFER_LISTS_COMPLETE NdisprotSendComplete;





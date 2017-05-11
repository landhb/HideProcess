#include "driver.h"

// tdiinfo.h was giving me a compiler warning for an unamed union
// so I defined TDIObjectID and TDIEntityID directly
typedef unsigned long ulong;

//* Structure of an entity ID.
typedef struct TDIEntityID {
	ulong		tei_entity;
	ulong		tei_instance;
} TDIEntityID;

//* Structure of an object ID.
typedef struct TDIObjectID {
	TDIEntityID	toi_entity;
	ulong		toi_class;
	ulong		toi_type;
	ulong		toi_id;
} TDIObjectID;


// Define a type to reference the old device control function we will be 
// replacing. We need to keep it around because we'll be handing off the request 
// handling to this function for requests we're not interested in masking
typedef NTSTATUS(*OLDIRPMJDEVICECONTROL)(IN PDEVICE_OBJECT, IN PIRP);
OLDIRPMJDEVICECONTROL OldIrpMjDeviceControl;


/*------------------------------------------------------------------------------
//								TDIObjectID Mappings
//------------------------------------------------------------------------------

The input buffer recieved by the TCPIP.sys driver should conform to the TDIObjectID 
structure. The TDIObjectID contains a part of the TCP_REQUEST_QUERY_INFORMATION_EX 
structure that is used with the IOCTL_TCP_QUERY_INFORMATION_EX control code to 
specify the kind of information being requested from the TCP driver.

typedef struct {
	TDIEntityID   toi_entity;
	unsigned long toi_class;
	unsigned long toi_type;
	unsigned long toi_id;
} TDIObjectID;

Reference: https://msdn.microsoft.com/en-us/library/bb432493(v=vs.85).aspx

*/

//------------------------------------------------------------------------------
//								toi_entity values
//------------------------------------------------------------------------------

// TCP value entity in TDIObject: inputBuffer->toi_entity
#define CO_TL_ENTITY 0x400

// UDP value entity in TDIObject: inputBuffer->toi_entity
#define CL_TL_ENTITY 0x401


//------------------------------------------------------------------------------
//								toi_id values
//------------------------------------------------------------------------------
// TDObjectID can be one of three supported toi_id values 0x101, 0x102, or 0x110
// Each value indicates a slightly different structure, we must map the 
// inputbuffer to the correct structure to parse it properly.


// inputBuffer->toi_id == 0x101
typedef struct _CONNINFO101 {
	unsigned long status;
	unsigned long src_addr;
	unsigned short src_port;
	unsigned short unk1;
	unsigned long dst_addr;
	unsigned short dst_port;
	unsigned short unk2;
} CONNINFO101, *PCONNINFO101;

// inputBuffer->toi_id == 0x102
typedef struct _CONNINFO102 {
	unsigned long status;
	unsigned long src_addr;
	unsigned short src_port;
	unsigned short unk1;
	unsigned long dst_addr;
	unsigned short dst_port;
	unsigned short unk2;
	unsigned long pid;
} CONNINFO102, *PCONNINFO102;

// inputBuffer->toi_id == 0x110
typedef struct _CONNINFO110 {
	unsigned long size;
	unsigned long status;
	unsigned long src_addr;
	unsigned short src_port;
	unsigned short unk1;
	unsigned long dst_addr;
	unsigned short dst_port;
	unsigned short unk2;
	unsigned long pid;
	PVOID    unk3[35];
} CONNINFO110, *PCONNINFO110;

typedef struct _REQINFO {
	PIO_COMPLETION_ROUTINE OldCompletion;
	unsigned long          ReqType;
} REQINFO, *PREQINFO;


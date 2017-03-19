#include "driver.h"

// Default IRP dispatcher, passthrough no action, return STATUS_SUCCESS
NTSTATUS defaultIrpHandler(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP IrpMessage) {

	UNREFERENCED_PARAMETER(DeviceObject);

	// Set status as success
	IrpMessage->IoStatus.Status = STATUS_SUCCESS;
	IrpMessage->IoStatus.Information = 0;

	// Complete request
	IoCompleteRequest(IrpMessage, IO_NO_INCREMENT);

	return(STATUS_SUCCESS);
}

// Handler to recieve IRP request and call Rootkit functionality
NTSTATUS IrpCallRootkit(_In_ PDEVICE_OBJECT DeviceObject, _In_ PIRP Irp) {

}
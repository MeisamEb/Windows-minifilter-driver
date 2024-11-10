#include "Device.h"


extern LOG_MANAGEMENT logmng;

NTSTATUS Function_IRP_MJ_CREATE(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    UNREFERENCED_PARAMETER(DeviceObject);
    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

NTSTATUS Function_IRP_MJ_CLOSE(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    UNREFERENCED_PARAMETER(DeviceObject);
    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

NTSTATUS Function_IRP_MJ_READ(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    UNREFERENCED_PARAMETER(DeviceObject);
    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

NTSTATUS Function_IRP_MJ_WRITE(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    UNREFERENCED_PARAMETER(DeviceObject);
    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}



NTSTATUS MyDeviceControl(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS status = STATUS_SUCCESS;
    ULONG info = 0;

    DbgPrint("MyDeviceControl: IOCTL code 0x%X\n", stack->Parameters.DeviceIoControl.IoControlCode);

    switch (stack->Parameters.DeviceIoControl.IoControlCode) {
    case IOCTL_HELLO:
        ExAcquireFastMutex(&LogMutex);

        if (LogBufferLength > 0) {
            ULONG copyLength = min(stack->Parameters.DeviceIoControl.OutputBufferLength, LogBufferLength);
            RtlCopyMemory(Irp->AssociatedIrp.SystemBuffer, LogBuffer, copyLength);
            info = copyLength;
            LogBufferLength = 0;
        }
        else {
            status = STATUS_NO_MORE_ENTRIES;
        }

        ExReleaseFastMutex(&LogMutex);
        break;

    default:
        status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = info;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    DbgPrint("MyDeviceControl: Completed with status 0x%X\n", status);

    return status;
}

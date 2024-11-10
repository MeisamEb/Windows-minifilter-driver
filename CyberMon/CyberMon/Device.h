#ifndef DEVICE_H
#define DEVICE_H

#include "Driver.h"

NTSTATUS MyDeviceControl(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS Function_IRP_MJ_CREATE(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS Function_IRP_MJ_CLOSE(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS Function_IRP_MJ_READ(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS Function_IRP_MJ_WRITE(PDEVICE_OBJECT DeviceObject, PIRP Irp);

#endif // DEVICE_H

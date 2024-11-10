#ifndef DRIVER_H
#define DRIVER_H

#include <fltKernel.h>
#include <dontuse.h>
#include <suppress.h>
#include <ntddk.h>
#include <wdm.h>
#include <Ntstrsafe.h>


#define FILE_NAME_BUFFER_SIZE 256
#define SIOCTL_TYPE 40000
#define IOCTL_HELLO CTL_CODE(SIOCTL_TYPE, 0x800, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)

extern PFLT_FILTER FilterHandle;
extern UNICODE_STRING deviceNameBuffer;
extern UNICODE_STRING deviceSymLinkBuffer;
extern FAST_MUTEX LogMutex;
extern WCHAR LogBuffer[1024];
extern ULONG LogBufferLength;
extern PDEVICE_OBJECT deviceObject;
//

typedef struct _LOG_MANAGEMENT {
    WCHAR EventType[15];
    //UNICODE_STRING PipeName;
    WCHAR PipeName[260];
    CHAR ImageName[50];
    ULONG ProcessId;
} LOG_MANAGEMENT, * PLOG_MANAGEMENT;


NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath);
NTSTATUS DeviceDriverUnload(PDRIVER_OBJECT DriverObject);

#endif // DRIVER_H

#include "Driver.h"
#include "MiniFilter.h"
#include "Device.h"

PFLT_FILTER FilterHandle = NULL;
UNICODE_STRING deviceNameBuffer = RTL_CONSTANT_STRING(L"\\Device\\MYDEVICE");
UNICODE_STRING deviceSymLinkBuffer = RTL_CONSTANT_STRING(L"\\DosDevices\\MyDevice");
FAST_MUTEX LogMutex;
WCHAR LogBuffer[1024];
LOG_MANAGEMENT logmng;
ULONG LogBufferLength = 0;
PDEVICE_OBJECT deviceObject = NULL;
PKEVENT g_StopEvent;

CONST FLT_OPERATION_REGISTRATION Callbacks[] = {
    { IRP_MJ_CREATE, 0, MiniFilterPreOperation, NULL },
    { IRP_MJ_READ, 0, MiniFilterPreOperation, NULL },
    { IRP_MJ_WRITE, 0, MiniFilterPreOperation, NULL },
    { IRP_MJ_CREATE_NAMED_PIPE, 0, MiniFilterPreOperation, NULL },
    { IRP_MJ_OPERATION_END }
};

CONST FLT_REGISTRATION FilterRegistration = {
    sizeof(FLT_REGISTRATION),               // Size
    FLT_REGISTRATION_VERSION,               // Version
    FLTFL_REGISTRATION_SUPPORT_NPFS_MSFS,   // Flags: Added the flag for NPFS and MSFS support
    NULL,                                   // Context
    Callbacks,                              // Operation callbacks
    MiniFilterUnload,                       // MiniFilterUnload
    InstanceSetup,                          // InstanceQueryTeardown
    NULL,                                   // InstanceTeardownStart
    NULL,                                   // InstanceTeardownComplete
    NULL,                                   // GenerateFileName
    NULL,                                   // NormalizeNameComponent
    NULL,                                   // NormalizeContextCleanup
    NULL,                                   // TransactionNotification
    NULL,                                   // NormalizeNameComponentEx
};

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
    NTSTATUS status;

    UNREFERENCED_PARAMETER(RegistryPath);

    // Register the filter
    status = FltRegisterFilter(DriverObject, &FilterRegistration, &FilterHandle);
    if (NT_SUCCESS(status)) {
        status = FltStartFiltering(FilterHandle);
        if (!NT_SUCCESS(status)) {
            FltUnregisterFilter(FilterHandle);
            FilterHandle = NULL;
            DbgPrint("Failed to start filtering: 0x%08X\n", status);
            return status;
        }
    }
    else {
        DbgPrint("Failed to register filter: 0x%08X\n", status);
        return status;
    }

    ExInitializeFastMutex(&LogMutex);

    // Create device
    status = IoCreateDevice(DriverObject, 0, &deviceNameBuffer, FILE_DEVICE_UNKNOWN, 0, FALSE, &deviceObject);
    if (!NT_SUCCESS(status)) {
        DbgPrint("Failed to create device. Error: 0x%08X\n", status);
        return status;
    }
    else {
        DbgPrint("Device created successfully.\n");
    }

    // Create symbolic link
    status = IoCreateSymbolicLink(&deviceSymLinkBuffer, &deviceNameBuffer);
    if (!NT_SUCCESS(status)) {
        IoDeleteDevice(deviceObject);
        deviceObject = NULL;
        DbgPrint("Failed to create symbolic link. Error: 0x%08X\n", status);
        return status;
    }
    else {
        DbgPrint("Symbolic link created successfully.\n");
    }


    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = MyDeviceControl;
    DriverObject->MajorFunction[IRP_MJ_CREATE] = Function_IRP_MJ_CREATE;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = Function_IRP_MJ_CLOSE;
    DriverObject->MajorFunction[IRP_MJ_READ] = Function_IRP_MJ_READ;
    DriverObject->MajorFunction[IRP_MJ_WRITE] = Function_IRP_MJ_WRITE;

    DriverObject->DriverUnload = DeviceDriverUnload;

    DbgPrint("Loading driver successfully.\n");

    return STATUS_SUCCESS;
}



NTSTATUS DeviceDriverUnload(PDRIVER_OBJECT DriverObject) {
    UNREFERENCED_PARAMETER(DriverObject);



    // Unregister the filter
    if (FilterHandle != NULL) {
        FltUnregisterFilter(FilterHandle);
        FilterHandle = NULL;
    }

    // Delete the symbolic link
    UNICODE_STRING symbolicLink = RTL_CONSTANT_STRING(L"\\DosDevices\\MyDevice");
    NTSTATUS status = IoDeleteSymbolicLink(&symbolicLink);
    if (!NT_SUCCESS(status)) {
        DbgPrint("Failed to delete symbolic link: 0x%08X\n", status);
    }

    // Delete the device object
    if (deviceObject != NULL) {
        IoDeleteDevice(deviceObject);
        deviceObject = NULL;
    }
    else {
        DbgPrint("No device object to delete.\n");
    }

    // Clean up the stop event
    if (g_StopEvent) {
        ObDereferenceObject(g_StopEvent);
    }

    DbgPrint("Device driver unloaded successfully.\n");

    return STATUS_SUCCESS;
}

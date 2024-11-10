#include "MiniFilter.h"

#define _countof(array) (sizeof(array) / sizeof((array)[0]))

LOG_MANAGEMENT logmng;
NTSTATUS MiniFilterUnload(FLT_FILTER_UNLOAD_FLAGS Flags) {
    UNREFERENCED_PARAMETER(Flags);

    if (FilterHandle != NULL) {
        FltUnregisterFilter(FilterHandle);
        FilterHandle = NULL;
    }

    DbgPrint("MiniFilter unloaded successfully.\n");

    return STATUS_SUCCESS;
}

NTSTATUS InstanceSetup(PCFLT_RELATED_OBJECTS FltObjects, FLT_INSTANCE_SETUP_FLAGS Flags, DEVICE_TYPE VolumeDeviceType, FLT_FILESYSTEM_TYPE VolumeFilesystemType) {
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(Flags);
    UNREFERENCED_PARAMETER(VolumeDeviceType);
    UNREFERENCED_PARAMETER(VolumeFilesystemType);

    DbgPrint("InstanceSetup: Volume %p, VolumeDeviceType %d, VolumeFilesystemType %d\n", FltObjects->Volume, VolumeDeviceType, VolumeFilesystemType);

    return STATUS_SUCCESS;
}

FLT_PREOP_CALLBACK_STATUS MiniFilterPreOperation(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext) {
    NTSTATUS status;
    PFLT_FILE_NAME_INFORMATION FileNameInfo;

    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);
    
    RtlZeroMemory(&logmng, sizeof(LOG_MANAGEMENT));

    if (Data->Iopb->MajorFunction == IRP_MJ_CREATE || Data->Iopb->MajorFunction == IRP_MJ_CREATE_NAMED_PIPE) {

        status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &FileNameInfo);
        if (!NT_SUCCESS(status)) {
            status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED, &FileNameInfo);
        }
        if (NT_SUCCESS(status)) {
            FltParseFileNameInformation(FileNameInfo);

            UNICODE_STRING pipePrefix;
            RtlInitUnicodeString(&pipePrefix, L"\\Device\\NamedPipe\\");

            if (RtlPrefixUnicodeString(&pipePrefix, &FileNameInfo->Name, TRUE)) {
                if (Data->Iopb->MajorFunction == IRP_MJ_CREATE) {
                    wcscpy_s(logmng.EventType, _countof(logmng.EventType), L"ConnectPipe");

                }
                else if (Data->Iopb->MajorFunction == IRP_MJ_CREATE_NAMED_PIPE) {
                    wcscpy_s(logmng.EventType, _countof(logmng.EventType), L"CreatePipe");
                    
                }

                PEPROCESS process;
                HANDLE processId = PsGetCurrentProcessId();
                wcscpy_s(logmng.PipeName, _countof(logmng.PipeName),FileNameInfo->FinalComponent.Buffer);

                if (NT_SUCCESS(PsLookupProcessByProcessId(processId, &process))) {
                    PCHAR processImageName = PsGetProcessImageFileName(process);
                    strncpy_s(logmng.ImageName, _countof(logmng.ImageName), processImageName, _TRUNCATE);
                    logmng.ProcessId = (ULONG)(ULONG_PTR)processId;

                    ExAcquireFastMutex(&LogMutex);

                    //writing to the log buffer
                    RtlCopyMemory(LogBuffer, &logmng, sizeof(LOG_MANAGEMENT));
                    LogBufferLength = sizeof(LOG_MANAGEMENT);

                    ExReleaseFastMutex(&LogMutex);

                    ObDereferenceObject(process);
                }
                else {
                    DbgPrint("Failed to lookup process by ID: %lu\n", (ULONG_PTR)processId);
                }
                DbgPrint("ProcessID: %lu\n", (ULONG_PTR)processId);
            }

            FltReleaseFileNameInformation(FileNameInfo);
        }
    }

    return FLT_PREOP_SUCCESS_NO_CALLBACK;
}


#include <stdio.h>
#include <windows.h>
#include <tchar.h>
#include <thread>
#include <memory>
#include <cstring>
#include <string>

// Device type
#define SIOCTL_TYPE 40000

// The IOCTL function codes from 0x800 to 0xFFF are for customer use.
#define IOCTL_HELLO CTL_CODE(SIOCTL_TYPE, 0x800, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)


//Kernel
typedef struct _LOG_MANAGEMENT {
    WCHAR EventType[15]{ 0 };
    WCHAR PipeName[260]{ 0 };
    CHAR ImageName[50]{ 0 };
    ULONG ProcessId{ 0 };
} LOG_MANAGEMENT, * PLOG_MANAGEMENT;


struct stCreatePipe
{
    std::wstring PipeName;
    std::string ImageName;
    uint32_t ProcessId{ 0 };
};

struct stConnectPipe
{
    std::wstring PipeName;
    std::string ImageName;
    uint32_t ProcessId{ 0 };
};

int main() {
    HANDLE hDevice;
    DWORD dwBytesRead;
    CHAR ReadBuffer[1024];
    PLOG_MANAGEMENT bufferAddress;

    hDevice = CreateFile(L"\\\\.\\MyDevice", GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hDevice == INVALID_HANDLE_VALUE) {
        printf("Failed to open device. Error: %d\n", GetLastError());
        return 1;
    }

    BOOL running = TRUE;
    while (running) {
        BOOL result = DeviceIoControl(hDevice, IOCTL_HELLO, NULL, 0, ReadBuffer, sizeof(ReadBuffer), &dwBytesRead, NULL);
        if (result) {
            if (dwBytesRead >= sizeof(LOG_MANAGEMENT)) {
                bufferAddress = (PLOG_MANAGEMENT)ReadBuffer;

                printf("CyberUser Log:\n");

                if (bufferAddress->EventType == L"CreatePipe")
                {
                    stCreatePipe CrtPipe;
                    _tprintf(_T(" EventType: %s\n"), bufferAddress->EventType);
                    CrtPipe.ImageName = std::string(bufferAddress->ImageName);
                    CrtPipe.PipeName = std::wstring(bufferAddress->PipeName);
                    CrtPipe.ProcessId = bufferAddress->ProcessId;

                    printf(" ImageName: %s\n", bufferAddress->ImageName);
                    _tprintf(_T(" PipeName: %s\n"), bufferAddress->PipeName);
                    _tprintf(_T(" ProcessId: %d\n\n"), bufferAddress->ProcessId);
                }
                else
                {
                    stConnectPipe ConPipe;

                    ConPipe.ImageName = std::string(bufferAddress->ImageName);
                    ConPipe.PipeName = std::wstring(bufferAddress->PipeName);
                    ConPipe.ProcessId = bufferAddress->ProcessId;

                    _tprintf(_T(" EventType: %s\n"), bufferAddress->EventType);
                    printf(" ImageName: %s\n", bufferAddress->ImageName);
                    _tprintf(_T(" PipeName: %s\n"), bufferAddress->PipeName);
                    _tprintf(_T(" ProcessId: %d\n\n"), bufferAddress->ProcessId);
                }
            }
            else {
                printf("Unexpected size of data read: %lu\n", dwBytesRead);
            }
        }
        else {
            DWORD error = GetLastError();
            if (error != ERROR_NO_MORE_ITEMS) {
                printf("DeviceIoControl failed. Error: %d\n", error);
                running = FALSE; // Stop the loop on error
            }
        }
    
    }

    CloseHandle(hDevice);
    return 0;
}


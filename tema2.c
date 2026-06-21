// Tema 2 ASO

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib, "advapi32.lib")

const char* getServiceState(DWORD state)
{
    switch (state)
    {
        case SERVICE_RUNNING:        return "RUNNING";
        case SERVICE_STOPPED:        return "STOPPED";
        case SERVICE_PAUSED:         return "PAUSED";
        case SERVICE_START_PENDING:  return "START_PENDING";
        case SERVICE_STOP_PENDING:   return "STOP_PENDING";
        default:                     return "UNKNOWN";
    }
}

int main()
{
    // deschidem Service Control Manager cu drept de enumerare
    SC_HANDLE scManager = OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);

    if (scManager == NULL)
    {
        DWORD err = GetLastError();
        fprintf(stderr, "Nu s-a putut deschide Service Manager. Eroare: %lu\n", err);
        fprintf(stderr, "(Incearca sa rulezi ca Administrator)\n");
        return 1;
    }

    DWORD bytesNeeded   = 0;
    DWORD totalServices = 0;
    DWORD resumeHandle  = 0;

    // primul apel - doar ca sa aflam cati bytes avem nevoie pentru buffer
    EnumServicesStatusEx(
        scManager,
        SC_ENUM_PROCESS_INFO,
        SERVICE_WIN32,
        SERVICE_STATE_ALL,
        NULL,
        0,
        &bytesNeeded,
        &totalServices,
        &resumeHandle,
        NULL
    );

    // alocam bufferul cu dimensiunea corecta
    BYTE* buffer = (BYTE*)malloc(bytesNeeded);
    if (buffer == NULL)
    {
        fprintf(stderr, "Eroare la alocare memorie\n");
        CloseServiceHandle(scManager);
        return 1;
    }

    LPENUM_SERVICE_STATUS_PROCESS serviceList =
        (LPENUM_SERVICE_STATUS_PROCESS)buffer;

    // al doilea apel - de data asta chiar citim datele
    BOOL ok = EnumServicesStatusEx(
        scManager,
        SC_ENUM_PROCESS_INFO,
        SERVICE_WIN32,
        SERVICE_STATE_ALL,
        buffer,
        bytesNeeded,
        &bytesNeeded,
        &totalServices,
        &resumeHandle,
        NULL
    );

    if (!ok)
    {
        fprintf(stderr, "EnumServicesStatusEx a esuat. Eroare: %lu\n", GetLastError());
        free(buffer);
        CloseServiceHandle(scManager);
        return 1;
    }

    int runningCount = 0;

    printf("========================================\n");
    printf("   Servicii sistem - stare curenta      \n");
    printf("========================================\n");
    printf("Total servicii gasite: %lu\n", totalServices);
    printf("----------------------------------------\n");

    DWORD i;
    for (i = 0; i < totalServices; i++)
    {
        DWORD currentState = serviceList[i].ServiceStatusProcess.dwCurrentState;

        // afisam doar serviciile care ruleaza efectiv
        if (currentState == SERVICE_RUNNING)
        {
            DWORD pid = serviceList[i].ServiceStatusProcess.dwProcessId;

            wprintf(L"[%lu] %s  (PID: %lu)\n", i + 1, serviceList[i].lpDisplayName, pid);

            runningCount++;
        }
    }

    printf("----------------------------------------\n");
    printf("Servicii active: %d / %lu\n", runningCount, totalServices);
    printf("========================================\n");

    free(buffer);
    CloseServiceHandle(scManager);

    system("pause");
    return 0;
}

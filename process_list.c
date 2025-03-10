#include <windows.h>
#include <stdio.h>
#include <psapi.h>

int main() {
    DWORD processes[1024], cbNeeded, cProcesses;

    SetConsoleOutputCP(CP_UTF8);

    if (!EnumProcesses(processes, sizeof(processes), &cbNeeded)) {
        printf("Failed to enumerate processes\n");
        return 1;
    }

    cProcesses = cbNeeded / sizeof(DWORD);

    printf("\"PID\",\"Process Name\",\"Bits\"\n");

    for (unsigned int i = 0; i < cProcesses; i++) {
        if (processes[i] != 0) {
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processes[i]);
            if (hProcess != NULL) {
                wchar_t szProcessName[MAX_PATH] = L"<unknown>";
                HMODULE hMod;
                DWORD cbNeededMod;
                BOOL isWow64 = FALSE;

                if (!IsWow64Process(hProcess, &isWow64)) {
                    isWow64 = FALSE;
                }

                if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeededMod)) {
                    GetModuleBaseNameW(hProcess, hMod, szProcessName, sizeof(szProcessName)/sizeof(wchar_t));
                }

                char utf8Name[1024] = {0};
                WideCharToMultiByte(CP_UTF8, 0, szProcessName, -1, utf8Name, sizeof(utf8Name), NULL, NULL);

                printf("\"%lu\",\"", processes[i]);
                for (char *p = utf8Name; *p; p++) {
                    if (*p == '"') printf("\"\"");
                    else printf("%c", *p);
                }
                printf("\",\"%s\"\n", isWow64 ? "32" : "64");

                CloseHandle(hProcess);
            }
        }
    }

    return 0;
}

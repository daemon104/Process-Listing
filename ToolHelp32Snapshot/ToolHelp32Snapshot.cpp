#include <windows.h>
#include <iostream>
#include <stdio.h>
#include <tchar.h>
#include <psapi.h>
#include <tlhelp32.h>
#define MAX 260

using namespace std;


// Quick sort algorithm
// Swap 2 number
void swap(DWORD* a, DWORD* b)
{
    DWORD t = *a;
    *a = *b;
    *b = t;
}

// Partiton function
int partition(DWORD arr[], int low, int high)
{
    DWORD pivot = arr[high];
    int i = (low - 1);

    for (int j = low; j <= high - 1; j++)
    {
        if (arr[j] < pivot)
        {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}

// Main sorting function
void quickSort(DWORD arr[], int low, int high)
{
    if (low < high)
    {
        DWORD pi = partition(arr, low, high);
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}

// Set process privilege
bool SetProcessPrivilege(
    HANDLE hToken, // Access token handle
    LPCTSTR lpszPrivilege, // Privilege name
    BOOL bEnablePrivilege) // Switch to enable or disable privilege
{
    TOKEN_PRIVILEGES tp; // Token privilege
    LUID luid;  // Local unique identifier

    // Lookup privilege value
    if (!LookupPrivilegeValue(NULL, lpszPrivilege, &luid))
    {
        cout << "Look up privilege value failed: " << GetLastError() << endl;
        return false;
    }
    else
    {
        cout << "Successfully look up privilege value!" << endl;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;

    if (bEnablePrivilege)
    {
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    }
    else
    {
        tp.Privileges[0].Attributes = 0;
    }

    // Adjust token privileges to enable or disable all privileges
    if (!AdjustTokenPrivileges(hToken, TRUE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL))
    {
        cout << "Adjust token privileges failed: " << GetLastError() << endl;
        return false;
    }
    else
    {
        cout << "Successfully adjust token privilege!" << endl;
        return true;
    }

}

// Get processes information (Name : ID : Number of Handles : Number of Threads)
void getProcessInfor(DWORD pid, HANDLE hSnapshot_Thread, HANDLE hSnapshot_Process)
{
    HANDLE hSnapshot_Module = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
    TCHAR ProcessName[MAX]; // Process name
    DWORD  thdCounter = 0, hndCounter = 0; // Thread, Handle counter
    LONG pPriority = 0; // Process priority

    PROCESSENTRY32 pEntry; // Process entry
    THREADENTRY32 tEntry; // Thread entry
    MODULEENTRY32 mEntry; // Module entry

    pEntry.dwSize = sizeof(pEntry); // Set size of process entry
    tEntry.dwSize = sizeof(tEntry); // Set size of thread entry
    mEntry.dwSize = sizeof(mEntry); // Set size of module entry

    // Get process's name, base priority, thread count
    if (hSnapshot_Process != NULL) // If process snapshot is valid
    {
        if (Process32First(hSnapshot_Process, &pEntry)) // If process is valid
        {
            do
            {
                if (pEntry.th32ProcessID == pid) // Specified process
                {
                    pPriority = pEntry.pcPriClassBase;
                    thdCounter = pEntry.cntThreads;

                    for (int i = 0; i < sizeof(pEntry.szExeFile); i++)
                    {
                        ProcessName[i] = pEntry.szExeFile[i];
                    }
                    break;
                }
            } while (Process32Next(hSnapshot_Process, &pEntry)); // Loop until there is no more process
        }
    }
    else
    {
        cout << "Process snapshot is invalid!" << endl;
    }

    // Get thread infor
    if (hSnapshot_Thread != NULL)
    {
        if (Thread32First(hSnapshot_Thread, &tEntry)) // If thread is valid
        {
            do
            {
                if (tEntry.dwSize >= FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) + sizeof(tEntry.th32OwnerProcessID)
                    && tEntry.th32OwnerProcessID == pid) // If thread belongs to process with specified ID
                {

                }
                tEntry.dwSize = sizeof(tEntry);
            } while (Thread32Next(hSnapshot_Thread, &tEntry)); // Loop until there is no more thread of specified process
        }
    }
    else
    {
        cout << "Thread snapshot is invalid!" << endl;
    }

    // Get module infor
    if (hSnapshot_Module != NULL) // If module snapshot is valid
    {
        if (Module32First(hSnapshot_Module, &mEntry))
        {
            do
            {
                hndCounter++;
                mEntry.dwSize = sizeof(mEntry);


            } while (Module32Next(hSnapshot_Module, &mEntry));
        }
    }
    else
    {
        cout << "Module snapshot is invalid!" << endl;
    }

    // Print process information
    cout << "===========================================================" << endl;
    if (pid == 0)
    {
        cout << "Image Name: System Idle Process" << "  |  PID: 0" << endl;
    }
    else
    {
        cout << "Image Name: " << ProcessName << "  |  PID: " << pid << endl;
    }
    cout << "    Priority: " << pPriority << endl
        << "    Number of Threads: " << thdCounter << endl
        << "    Number of Handles: " << hndCounter << endl;
}

int main()
{
    // Change access token of this process to enter debug mode (Debug privilege)
    HANDLE hToken = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, GetCurrentProcessId()); // Access token handle
    OpenProcessToken(hToken, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken); // Open token handle
    bool Debug = SetProcessPrivilege(hToken, SE_DEBUG_NAME, TRUE); // Enable debug privilege

    if (Debug == false)
    {
        cout << "Enable debug privilege failed: " << GetLastError() << endl;
        return 1;
    }
    else
    {
        cout << "Successfully enable debug privilege!\n" << endl;
        CloseHandle(hToken);
    }


    // Start gathering process information
    HANDLE hSnapshot_Thread = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0); // Snapshot thread handle
    HANDLE hSnapshot_Process = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); // Snapshot process handle
    DWORD pid[1024], bytes_rcv, number_of_PID;
    int max, result = 0;

    result = EnumProcesses(pid, sizeof(pid), &bytes_rcv);

    if (result == 0)
    {
        cout << "EnumProcesses failed\n" << GetLastError();
        return 1;
    }

    // Calculate the number of PIDs
    number_of_PID = bytes_rcv / sizeof(DWORD);

    // Sorting PIDs
    quickSort(pid, 0, number_of_PID - 1);

    // Print process information
    for (int i = 0; i < number_of_PID; i++)
    {
        getProcessInfor(pid[i], hSnapshot_Thread, hSnapshot_Process);
    }

    CloseHandle(hSnapshot_Thread);
    CloseHandle(hSnapshot_Process);
    return 0;
}
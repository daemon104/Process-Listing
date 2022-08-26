#define WIN32_LEAN_AND_MEAN 1

#include <windows.h>
#include <iostream>
#include <stdio.h>
#include <tchar.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <winternl.h>
#define MAX 260

using namespace std;


// Quick sort algorithm
void swap(DWORD* a, DWORD* b)
{
    DWORD t = *a;
    *a = *b;
    *b = t;
}

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

void quickSort(DWORD arr[], int low, int high)
{
    if (low < high)
    {
        DWORD pi = partition(arr, low, high);
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}

void ErrorMessage(DWORD ErrorID) 
{
    wchar_t ErrorBuffer[MAX];
    FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, ErrorID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        ErrorBuffer, (sizeof(ErrorBuffer) / sizeof(wchar_t)), NULL);
    wcout << ErrorBuffer;
}

// Set process privilege
bool SetProcessPrivilege(
    HANDLE hToken, 
    LPCSTR lpszPrivilege, 
    BOOL bEnablePrivilege)
{
    TOKEN_PRIVILEGES tp;
    LUID luid;

    // Lookup privilege value
    if (!LookupPrivilegeValueA(NULL, lpszPrivilege, &luid))
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
        cout << "Adjust token privileges failed: ";
        ErrorMessage(GetLastError());
        return false;
    }
    else
    {
        cout << "Successfully adjust token privilege!" << endl;
        return true;
    }

}

// Get processes information (Name : ID : Number of Handles : Number of Threads)
void getProcessInfor(DWORD pid)
{
   HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
   
   if (hProcess != NULL) {
       HMODULE hMod;
       DWORD cbNeeded;
       if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded)) {
		  TCHAR szModName[MAX];
		  if (GetModuleBaseName(hProcess, hMod, szModName, sizeof(szModName) / sizeof(TCHAR))) {
              wcout << "PID: " << pid << " - Process Name: " << szModName << endl;
		  }
	   }
   }
   else {
       cout << "PID: " << pid << " - ";
       ErrorMessage(GetLastError());
   }

   CloseHandle(hProcess);  
}

int main()
{
    // Change access token of this process to enter debug mode (Debug privilege)
    HANDLE hToken = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, GetCurrentProcessId()); 
    OpenProcessToken(hToken, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken); 
    bool Debug = SetProcessPrivilege(hToken, SE_DEBUG_NAME, TRUE); 

    if (Debug == false)
    {
        cout << "Enable debug privilege failed: ";
		ErrorMessage(GetLastError());
        return 1;
    }
    else
    {
        cout << "Successfully enable debug privilege!\n" << endl;
        CloseHandle(hToken);
    }

    // Start gathering process information
    DWORD pid[1024], bytes_rcv, number_of_PID;
    int max, result = 0;

    result = EnumProcesses(pid, sizeof(pid), &bytes_rcv);

    if (result == 0)
    {
        cout << "EnumProcesses failed: ";
        ErrorMessage(GetLastError());
        return 1;
    }

    // Calculate the number of PIDs
    number_of_PID = bytes_rcv / sizeof(DWORD);

    // Sorting PIDs
    quickSort(pid, 0, number_of_PID - 1);

    // Print process information

    cout << "===================\n"
        << "Process Enumeration\n"
        << "==================="
        << "\n\n";

    for (int i = 0; i < number_of_PID; i++)
    {
        getProcessInfor(pid[i]);
    }
	
    return 0;
}
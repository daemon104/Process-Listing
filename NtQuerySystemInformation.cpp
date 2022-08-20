#define WIN32_LEARN_AND_MEAN 1
#define UNICODE 1

#include <Windows.h>
#include <Shlwapi.h>
#include <winternl.h>
#include <iostream>
#include <wchar.h>
#include <stdlib.h>
#include <iomanip>
#include <WinBase.h>

#pragma comment(lib, "Ntdll.lib")
#pragma comment(lib, "Shlwapi.lib")

using namespace std;

constexpr unsigned int STATUS_SUCCESS = 0;
constexpr unsigned int STATUS_INFO_LENGTH_MISMATCH = 0x0C0000004;

template <typename T>
LPCWSTR GetReadableSize(T size) {
	LPCWSTR lpSize = (LPCWSTR)VirtualAlloc(NULL, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	if (StrFormatByteSizeW((LONGLONG)size, (PWSTR)lpSize, MAX_PATH) == NULL) {
		VirtualFree((LPVOID)lpSize, 0, MEM_RELEASE);
		lpSize = NULL;
	}
	return lpSize;
}

LPWSTR getComputerName(void) {

	DWORD Size = MAX_COMPUTERNAME_LENGTH + 1;
	LPWSTR lpHostName = (LPWSTR)VirtualAlloc(NULL, Size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	if (!GetComputerName(lpHostName, &Size)) {
		cout << "\nCannot get the computer name!!\n";
		VirtualFree(lpHostName, 0, MEM_RELEASE);
		lpHostName = NULL;
	}
	return lpHostName;
}

LPWSTR getUserName(void) {

	DWORD Size = MAX_COMPUTERNAME_LENGTH + 1;
	LPWSTR lpUserName = (LPWSTR)VirtualAlloc(NULL, Size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	if (!GetUserName(lpUserName, &Size)) {
		cout << "\nCannot get the user name!!\n";
		VirtualFree(lpUserName, 0, MEM_RELEASE);
		lpUserName = NULL;
	}
	return lpUserName;
}

void PrintProcessInfo(PSYSTEM_PROCESS_INFORMATION p)
{
	wcout << L"\nProcess information for " << getComputerName()
		<< L"-" << getUserName() << ":\n\n";
	
	cout << setfill(' ')
		<< setw(14) << left << "PID"
		<< setw(42) << left << "Image Name"
		<< setw(10) << left << "Handle"
		<< setw(10) << left << "Thread"
		<< setw(12) << left << "Priority"
		<< setw(16) << left << "Session ID"
		<< setw(16) << left << "VM size";
	cout << setfill('=')
		<< setw(120) << left << "" << endl;
	
	do {		
		wcout << setfill(L' ')
			<< setw(14) << left << HandleToULong(p->UniqueProcessId)
			<< setw(42) << left << (p->ImageName.Buffer ? p->ImageName.Buffer : L"Idle")
			<< setw(10) << left << p->HandleCount
			<< setw(10) << left << p->NumberOfThreads
			<< setw(12) << left << p->BasePriority
			<< setw(16) << left << p->SessionId
			<< setw(16) << left << GetReadableSize(p->VirtualSize)
			<< endl;
		
		p = (PSYSTEM_PROCESS_INFORMATION)((PBYTE)p + p->NextEntryOffset);
	} while (p->NextEntryOffset);
}


BOOL QueryProcessInformation(PSYSTEM_PROCESS_INFORMATION p)
{
	DWORD dwRet;
	DWORD dwSize = 0;
	NTSTATUS Status = STATUS_INFO_LENGTH_MISMATCH;
	
	while (true)
	{
		// Check if pointer p is not NULL then free it
		if (p != NULL) { VirtualFree(p, 0, MEM_RELEASE); }
		
		p = (PSYSTEM_PROCESS_INFORMATION)VirtualAlloc(NULL, dwSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

		// Query system to get the process list information
		Status = NtQuerySystemInformation(SystemProcessInformation, (PVOID)p, (ULONG)dwSize, &dwRet);

		if (Status == STATUS_SUCCESS)						
		{	
			cout << "Query process information successfully!!" << endl;
			break;			
		}
		else if (Status != STATUS_INFO_LENGTH_MISMATCH)		
		{	
			VirtualFree(p, 0, MEM_RELEASE);
			p = NULL;
			cout << "NtQuerySystemInformation failed with error code: " << Status << endl;
			return FALSE;
		}
		
		// Add more 16kb to buffer in case there is more process opened during this loop
		dwSize = dwRet + (2 << 14);
	}
	
	// Print process info
	PrintProcessInfo(p);

	// Free p after use
	VirtualFree(p, 0, MEM_RELEASE);
	p = NULL;
	return TRUE;
}

int main()
{
	BOOL check = FALSE; 
	PSYSTEM_PROCESS_INFORMATION p = NULL;
	
	cout << "Start gathering processes information...\n\n";

	check = QueryProcessInformation(p);
	
	return 0;
}
#include "psl.h"

using namespace std;

template <typename T>
LPCWSTR GetReadableSize(T size) {
	LPCWSTR lpSize = (LPCWSTR)VirtualAlloc(nullptr, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	if (StrFormatByteSizeW((LONGLONG)size, (PWSTR)lpSize, MAX_PATH) == NULL) {
		VirtualFree((LPVOID)lpSize, 0, MEM_RELEASE);
		lpSize = nullptr;
	}
	return lpSize;
}

LPWSTR getComputerName(void) {

	DWORD Size = MAX_COMPUTERNAME_LENGTH + 1;
	LPWSTR lpHostName = (LPWSTR)VirtualAlloc(nullptr, Size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	if (!GetComputerName(lpHostName, &Size)) {
		cout << "\nCannot get the computer name!!\n";
		VirtualFree(lpHostName, 0, MEM_RELEASE);
		lpHostName = nullptr;
	}
	return lpHostName;
}

LPWSTR getUserName(void) {

	DWORD Size = MAX_COMPUTERNAME_LENGTH + 1;
	LPWSTR lpUserName = (LPWSTR)VirtualAlloc(nullptr, Size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	if (!GetUserName(lpUserName, &Size)) {
		cout << "\nCannot get the user name!!\n";
		VirtualFree(lpUserName, 0, MEM_RELEASE);
		lpUserName = nullptr;
	}
	return lpUserName;
}

void PrintProcessInfo(PSYSTEM_PROCESS_INFORMATION p)
{
	PSYSTEM_PROCESS_INFORMATION temp = p;

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
			<< setw(14) << left << HandleToULong(temp->UniqueProcessId)
			<< setw(42) << left << (temp->ImageName.Buffer ? temp->ImageName.Buffer : L"Idle")
			<< setw(10) << left << temp->HandleCount
			<< setw(10) << left << temp->NumberOfThreads
			<< setw(12) << left << temp->BasePriority
			<< setw(16) << left << temp->SessionId
			<< setw(16) << left << GetReadableSize(temp->VirtualSize)
			<< endl;
		
		temp = (PSYSTEM_PROCESS_INFORMATION)((PBYTE)temp + temp->NextEntryOffset);
	} while (temp->NextEntryOffset);

	VirtualFree(temp, 0, MEM_RELEASE);
	temp = nullptr;
}

void PrintThreadInfo(PSYSTEM_PROCESS_INFORMATION p) 
{
	PSYSTEM_PROCESS_INFORMATION temp = p;

	do
	{
		wcout << L"\nProcess information for " << getComputerName()
			<< L"-" << getUserName() << ":\n\n"
			<< L"\n" << (temp->ImageName.Buffer ? temp->ImageName.Buffer : L"Idle")
			<< L" - " << HandleToULong(temp->UniqueProcessId) << "\n";

		cout << setfill(' ')
			<< "\t"
			<< setw(11) << left << "TID"
			<< setw(21) << left << "Base Priority"
			<< setw(24) << left << "Dynamic Priority"
			<< setw(13) << left << "State"
			<< endl;
		cout << setfill('=')
			<< "\t"
			<< setw(69) << left << "" << endl;

		PSYSTEM_THREAD_INFORMATION pt = PSYSTEM_THREAD_INFORMATION(temp + 1);

		for (int i = 0; i < temp->NumberOfThreads; i++)
		{
			string state = "";

			cout << setfill(' ')
				<< "\t"
				<< setw(11) << left << HandleToULong(pt->ClientId.UniqueThread)
				<< setw(21) << left << pt->BasePriority
				<< setw(24) << left << pt->Priority;
			
			switch (pt->ThreadState)
			{
			case 0:
				state = "Init";
				break;
			case 1:
				state = "Ready";
				break;
			case 2:
				state = "Running";
				break;
			case 3:
				state = "Standby";
				break;
			case 4:
				state = "Terminate";
				break;
			case 5:
				state = "Waiting";
				break;
			case 6:
				state = "Transition";
				break;
			case 7:
				state = "Deferred ready";
				break;
			default:
				state = "Unknown";
				break;
			}
			cout << setw(13) << left << state
				<< endl;
			pt++;
		}

		temp = (PSYSTEM_PROCESS_INFORMATION)((PBYTE)temp + temp->NextEntryOffset);
	} while (temp->NextEntryOffset);

	VirtualFree(temp, 0, MEM_RELEASE);
	temp = nullptr;
}

void PrintMemoryInfo(PSYSTEM_PROCESS_INFORMATION p)
{
	PSYSTEM_PROCESS_INFORMATION temp = p;

	wcout << L"\nProcess's memory information for " << getComputerName()
		<< L"-" << getUserName() << ":\n\n";

	cout << setfill(' ')
		<< setw(11) << left << "PID"
		<< setw(30) << left << "Page-file/Peak"
		<< setw(32) << left << "Working Set/Peak"
		<< setw(28) << left << "Virtual/Peak"
		<< setw(19) << left << "Private Page Count"
		<< endl;
	cout << setfill('=')
		<< setw(120) << left << "" << endl;

	do {

		wstring str1 = wstring(GetReadableSize(temp->PagefileUsage)) + L" / " + wstring(GetReadableSize(temp->PeakPagefileUsage));
		wstring str2 = wstring(GetReadableSize(temp->WorkingSetSize)) + L" / " + wstring(GetReadableSize(temp->PeakWorkingSetSize));
		wstring str3 = wstring(GetReadableSize(temp->VirtualSize)) + L" / " + wstring(GetReadableSize(temp->PeakVirtualSize));

		wcout << setfill(L' ')
			<< setw(11) << left << HandleToULong(temp->UniqueProcessId)
			<< setw(30) << left << str1
			<< setw(32) << left << str2
			<< setw(28) << left << str3
			<< setw(19) << left << GetReadableSize(temp->PrivatePageCount)
			<< endl;

		temp = (PSYSTEM_PROCESS_INFORMATION)((PBYTE)temp + temp->NextEntryOffset);
	} while (temp->NextEntryOffset);

	VirtualFree(temp, 0, MEM_RELEASE);
	temp = nullptr;
}

BOOL QueryProcessInformation(PSYSTEM_PROCESS_INFORMATION p)
{
	DWORD dwRet;
	DWORD dwSize = 0;
	NTSTATUS Status = STATUS_INFO_LENGTH_MISMATCH;
	
	while (true)
	{
		// Check if pointer p is not NULL then free it
		if (p != nullptr) { VirtualFree(p, 0, MEM_RELEASE); }
		
		p = (PSYSTEM_PROCESS_INFORMATION)VirtualAlloc(nullptr, dwSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

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
			p = nullptr;
			cout << "NtQuerySystemInformation failed with error code: " << Status << endl;
			return FALSE;
		}
		
		// Add more 16kb to buffer in case there is more process opened during this loop
		dwSize = dwRet + (2 << 14);
	}
	
	// Print process info
	PrintProcessInfo(p);

	// Print thread info
	//PrintThreadInfo(p);

	// Print process's memory info
	//PrintMemoryInfo(p);

	// Free p after use
	VirtualFree(p, 0, MEM_RELEASE);
	p = nullptr;
	return TRUE;
}

int main()
{
	BOOL check = FALSE; 
	PSYSTEM_PROCESS_INFORMATION p = nullptr;
	
	cout << "Start gathering processes information...\n\n";

	check = QueryProcessInformation(p);
	
	return 0;
}
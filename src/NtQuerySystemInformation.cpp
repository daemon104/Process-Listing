#include "psl.h"

using namespace std;


// Function to get readable size
template <typename T>
LPCWSTR GetReadableSize(T size) {
	LPCWSTR lpSize = (LPCWSTR)VirtualAlloc(nullptr, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	if (StrFormatByteSizeW((LONGLONG)size, (PWSTR)lpSize, MAX_PATH) == NULL) {
		VirtualFree((LPVOID)lpSize, 0, MEM_RELEASE);
		lpSize = nullptr;
	}
	return lpSize;
}


// Get computer name (host name)
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


// Get user name (the loggon user that run this program)
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


// Help page
void Manual()
{
	cout << "\n=================================="
		<< "\nBasic process enumeration program"
		<< "\n=================================="
		<< "\n\nThis program is use for enumerate some basic process information using NtQuerySystemInformation() function defined in Ntdll.dll. See more details in MSDN, docs of winternl.h"
		<< "\n\nList of arguments:\n\n"
		<< "\tdefault, --process: Print process information (All running process, including system-level process)"	
		<< "\n\t-t,  --thread: Print process's thread details"
		<< "\n\t-m, --memory: Print process's memory details"
		<< "\n\t-p, --pid <process pid>: Print information by specific pid"
		<< "\n\t-h, --help: Show help page"
		<< endl;	
}


// Print process info
void PrintProcessInfo(PSYSTEM_PROCESS_INFORMATION p)
{
	PSYSTEM_PROCESS_INFORMATION temp = p;

	wcout << L"\nProcess information for " << getComputerName()
		<< L"-" << getUserName() << ":\n\n";
	
	cout << setfill(' ')
		<< setw(14) << left << "PID"
		<< setw(42) << left << "Image Name"
		<< setw(10) << left << "Handles"
		<< setw(10) << left << "Threads"
		<< setw(12) << left << "Priority"
		<< setw(16) << left << "Session ID"
		<< setw(16) << left << "VM size"
		<< endl;
	cout << setfill('=')
		<< setw(119) << left << "" << endl;
	
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


// Print thread info
void PrintThreadInfo(PSYSTEM_PROCESS_INFORMATION p) 
{
	PSYSTEM_PROCESS_INFORMATION temp = p;

	wcout << L"\nProcess information for " << getComputerName()
		<< L"-" << getUserName() << ":\n\n";
	do
	{	
		wcout << L"\n" << (temp->ImageName.Buffer ? temp->ImageName.Buffer : L"Idle")
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


// Print memory info
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
		<< setw(119) << left << "" << endl;

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

// Print process info with specific pid
void PrintWithPID(PSYSTEM_PROCESS_INFORMATION p, int pid)
{
	PSYSTEM_PROCESS_INFORMATION temp = p;
	BOOL check = FALSE;

	do {

		if (HandleToULong(temp->UniqueProcessId) == (ULONG)pid)
		{
			check = TRUE;
			wcout << L"\nInformation for proccess with PID: " << HandleToULong(temp->UniqueProcessId)
				<< L"\n\tImage Name: " << (temp->ImageName.Buffer ? temp->ImageName.Buffer : L"Idle")
				<< L"\n\tNumber of handles: " << temp->HandleCount
				<< L"\n\tNumber of thread: " << temp->NumberOfThreads
				<< L"\n\tBase priority: " << temp->BasePriority
				<< L"\n\tSession ID: " << temp->SessionId
				<< L"\n\tVirtual memory size: " << GetReadableSize(temp->VirtualSize)
				<< endl;
			break;
		}

		temp = (PSYSTEM_PROCESS_INFORMATION)((PBYTE)temp + temp->NextEntryOffset);
	} while (temp->NextEntryOffset);

	if (!check) { cout << "\nThere is no running process with specified pid. Please check the input again...\n"; }

	VirtualFree(temp, 0, MEM_RELEASE);
	temp = nullptr;
}


int main(int argc, char* argv[])
{
	BOOL check = FALSE; 
	DWORD dwRet;
	DWORD dwSize = 0;
	PSYSTEM_PROCESS_INFORMATION p = nullptr;
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
			check = TRUE;
			break;
		}
		else if (Status != STATUS_INFO_LENGTH_MISMATCH)
		{
			VirtualFree(p, 0, MEM_RELEASE);
			p = nullptr;
			cout << "\nNtQuerySystemInformation failed with error code: " << Status << endl;
			check = FALSE;
			break;
		}

		// Add more 16kb to buffer in case there is more process opened during this loop
		dwSize = dwRet + (2 << 14);
	}


	if (check) 
	{
		if (argc == 1) 
		{
			PrintProcessInfo(p);
		}
		else if (argc == 2 || argc == 3 ) 
		{
			if (string(argv[1]) == "-h" || string(argv[1]) == "--help") 
			{
				Manual();
			}
			else if (string(argv[1]) == "-t" || string(argv[1]) == "--thread") 
			{
				PrintThreadInfo(p);
			}
			else if (string(argv[1]) == "-m" || string(argv[1]) == "--memory") 
			{
				PrintMemoryInfo(p);
			}
			else if ((string(argv[1]) == "-p" || string(argv[1]) == "--pid") && string(argv[2]) != "")
			{
				int pid = stoi(string(argv[2]));
				PrintWithPID(p, pid);
			}
		}
		else 
		{
			cout << "\nUse -h or --help to show help page.\n";
		}

		// Free p after use
		if (p != nullptr) 
		{	
			VirtualFree(p, 0, MEM_RELEASE);
			p = nullptr;
		}
	}
	else 
	{
		cout << "\nQuery process information failed. Something went wrong. Exiting the program...\n";
	}

	return 0;
}
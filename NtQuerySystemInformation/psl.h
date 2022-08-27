#define WIN32_LEARN_AND_MEAN 1
#define UNICODE 1
#define MAX 260

#include <Windows.h>
#include <Shlwapi.h>
//#include <winternl.h>
#include <iostream>
#include <wchar.h>
#include <stdlib.h>
#include <iomanip>
#include <WinBase.h>
#include <string.h>

#pragma once
#pragma comment(lib, "Ntdll.lib")
#pragma comment(lib, "Shlwapi.lib")

constexpr unsigned int STATUS_SUCCESS = 0;
constexpr unsigned int STATUS_INFO_LENGTH_MISMATCH = 0x0C0000004;


//****************************************************************
// These code below are for user to know details about what we will going to use in this program.
// All these structures are well defined in winternl.h so if you want to go futher and enumerate more information, you could uncomment
// the include line above.
//***************************************************************


typedef struct _UNICODE_STRING {
	USHORT Length;
	USHORT MaximumLength;
	PWSTR  Buffer;
} UNICODE_STRING, * PUNICODE_STRING;

typedef LONG KPRIORITY;

typedef struct _CLIENT_ID {
	HANDLE UniqueProcess;
	HANDLE UniqueThread;
} CLIENT_ID, * PClIENT_ID;

// Learn more about this struct in MSDN
typedef struct _SYSTEM_PROCESS_INFORMATION
{
	ULONG NextEntryOffset;
	ULONG NumberOfThreads;
	BYTE Reserved1[48];
	UNICODE_STRING ImageName;
	KPRIORITY BasePriority;
	HANDLE UniqueProcessId;
	PVOID Reserved2;
	ULONG HandleCount;
	ULONG SessionId;
	PVOID Reserved3;
	SIZE_T PeakVirtualSize;
	SIZE_T VirtualSize;
	ULONG Reserved4;
	SIZE_T PeakWorkingSetSize;
	SIZE_T WorkingSetSize;
	PVOID Reserved5;
	SIZE_T QuotaPagedPoolUsage;
	PVOID Reserved6;
	SIZE_T QuotaNonPagedPoolUsage;
	SIZE_T PagefileUsage;
	SIZE_T PeakPagefileUsage;
	SIZE_T PrivatePageCount;
	LARGE_INTEGER Reserved7[6];
} SYSTEM_PROCESS_INFORMATION, * PSYSTEM_PROCESS_INFORMATION;

typedef struct _SYSTEM_THREAD_INFORMATION {
	LARGE_INTEGER Reserved1[3];
	ULONG Reserved2;
	PVOID StartAddress;
	CLIENT_ID ClientId;
	KPRIORITY Priority;
	LONG BasePriority;
	ULONG Reserved3;
	ULONG ThreadState;
	ULONG WaitReason;
} SYSTEM_THREAD_INFORMATION, * PSYSTEM_THREAD_INFORMATION;

// There is more information class, some unavailable for now on, some still useful but not included here
typedef enum _SYSTEM_INFORMATION_CLASS {
	SystemBasicInformation = 0,
	SystemPerformanceInformation = 2,
	SystemTimeOfDayInformation = 3,
	SystemProcessInformation = 5,
	SystemProcessorPerformanceInformation = 8,
	SystemInterruptInformation = 23,
	SystemExceptionInformation = 33,
	SystemRegistryQuotaInformation = 37,
	SystemLookasideInformation = 45,
	SystemCodeIntegrityInformation = 103,
	SystemPolicyInformation = 134,
} SYSTEM_INFORMATION_CLASS;

// This is just for user to know all params of the the function, its implement in Ntdll libary
EXTERN_C NTSTATUS NTAPI NtQuerySystemInformation(
	IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
	OUT PVOID SystemInformation,
	IN ULONG SystemInformationLength,
	OUT PULONG ReturnLength OPTIONAL
);

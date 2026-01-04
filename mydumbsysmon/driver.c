#include <ntifs.h>
//#include <ntddk.h>
//#include <wdm.h>
#include <fltKernel.h>
#include <shared.h>

NTSTATUS MinifilterUnload(_In_ FLT_FILTER_UNLOAD_FLAGS Flags);

PFLT_FILTER g_FilterHandle = NULL;
PFLT_PORT g_ServerPort = NULL;
PFLT_PORT g_ClientPort = NULL;

const FLT_REGISTRATION FilterRegistration = {
	sizeof(FLT_REGISTRATION),
	FLT_REGISTRATION_VERSION,
	0,
	NULL,
	NULL,
	MinifilterUnload,
	NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

NTSTATUS PortConnectNotify(PFLT_PORT ClientPort, PVOID ServerPortCookie, PVOID ConnectionContext, ULONG SizeOfContext, PVOID *ConnectionPortCookie);
VOID PortDisconnectNotify(PVOID ConnectionCookie); 

VOID sCreateProcessNotifyRoutineEx(PEPROCESS Process, HANDLE pid, PPS_CREATE_NOTIFY_INFO CreateInfo) {
	UNREFERENCED_PARAMETER(Process);

	if (g_ClientPort == NULL) return; 

	SYSMON_EVENT event = { 0 };
	event.pid = pid;

	if (CreateInfo != NULL) {
		// DUMB EID 1: Process Create
		event.create = TRUE;
		event.ppid = CreateInfo->ParentProcessId;
		KdPrint(("[DUMBSYSMON] PID: %p\n", pid));
		KdPrint(("[DUMBSYSMON] PPID: %p\n", CreateInfo->ParentProcessId));

		if (CreateInfo->ImageFileName) {
			USHORT len = min(CreateInfo->ImageFileName->Length, 258 * sizeof(WCHAR));
			RtlCopyMemory(event.ImagePath, CreateInfo->ImageFileName->Buffer, len);
			KdPrint(("[DUMBSYSMON] Image: %wZ\n", CreateInfo->ImageFileName));
		}
		if (CreateInfo->CommandLine) {
			KdPrint(("[DUMBSYSMON] CommandLine: %wZ\n", CreateInfo->CommandLine));
		}
	}
	else {
		event.create = FALSE;
		KdPrint(("[DUMBSYSMON] Process Exit: PID %p\n", pid));
	}

	LARGE_INTEGER timeout;
	timeout.QuadPart = 0;
	FltSendMessage(g_FilterHandle, &g_ClientPort, &event, sizeof(SYSMON_EVENT), NULL, NULL, &timeout);
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
	UNREFERENCED_PARAMETER(RegistryPath);

	NTSTATUS status;
	OBJECT_ATTRIBUTES oa;
	UNICODE_STRING uniName = RTL_CONSTANT_STRING(L"\\DumbSysmonPort");
	PSECURITY_DESCRIPTOR sd;

	//DriverObject->DriverUnload = DriverUnload;

	status = FltRegisterFilter(DriverObject, &FilterRegistration, &g_FilterHandle);
	if (!NT_SUCCESS(status)) return status;

	status = FltBuildDefaultSecurityDescriptor(&sd, FLT_PORT_ALL_ACCESS);
	if (NT_SUCCESS(status)) {
		InitializeObjectAttributes(&oa, &uniName, OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE, NULL, sd);

		status = FltCreateCommunicationPort(g_FilterHandle, &g_ServerPort, &oa, NULL, PortConnectNotify, PortDisconnectNotify, NULL, 1);
		FltFreeSecurityDescriptor(sd);
	}

	if (NT_SUCCESS(status)) {
		status = PsSetCreateProcessNotifyRoutineEx(sCreateProcessNotifyRoutineEx, FALSE);
		if (!NT_SUCCESS(status)) {
			KdPrint(("[DUMBSYSMON] Failed to register callback for ProcessCreate(0x%08X).\n", status));
		}
		else {
			KdPrint(("[DUMBSYSMON] ProcessCreate Callback registered successfully.\n"));
		}
		status = FltStartFiltering(g_FilterHandle);
		if (!NT_SUCCESS(status)) {
			KdPrint(("[DUMBSYSMON] Failed to register FLTFiltering (0x%08X). \n", status));
		}
		else {
			KdPrint(("[DUMBSYSMON] FLTFiltering Callback registered successfully.\n"));
		}
	}
	
	return status;
}

NTSTATUS MinifilterUnload( FLT_FILTER_UNLOAD_FLAGS flags) {
	UNREFERENCED_PARAMETER(flags);

	KdPrint(("[DUMBSYSMON] Unloading the driver"));
	PsSetCreateProcessNotifyRoutineEx(sCreateProcessNotifyRoutineEx, TRUE);

	FltCloseCommunicationPort(g_ServerPort);
	FltUnregisterFilter(g_FilterHandle);
	return STATUS_SUCCESS;

}

NTSTATUS PortConnectNotify(PFLT_PORT ClientPort, PVOID ServerPortCookie, PVOID ConnectionContext, ULONG SizeOfContext, PVOID* ConnectionPortCookie) {
	UNREFERENCED_PARAMETER(ServerPortCookie);
	UNREFERENCED_PARAMETER(ConnectionContext);
	UNREFERENCED_PARAMETER(SizeOfContext);
	UNREFERENCED_PARAMETER(ConnectionPortCookie);

	g_ClientPort = ClientPort;
	return STATUS_SUCCESS;
}

VOID PortDisconnectNotify(PVOID ConnectionCookie) {
	UNREFERENCED_PARAMETER(ConnectionCookie);
	FltCloseClientPort(g_FilterHandle, &g_ClientPort);
}


#pragma once
typedef struct _SYSMON_EVENT {
	HANDLE pid;
	HANDLE ppid;
	BOOLEAN create;
	WCHAR ImagePath[MAXCHAR];
} SYSMON_EVENT, * PSYSMON_EVENT;
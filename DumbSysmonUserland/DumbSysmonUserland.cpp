// DumbSysmonUserland.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <windows.h>
#include <fltUser.h>
#include <shared.h>


#pragma comment(lib, "user32.lib")
#pragma comment(lib, "fltlib.lib")

int main()
{
	HRESULT hr;
	HANDLE port = INVALID_HANDLE_VALUE;

	hr = FilterConnectCommunicationPort(L"\\DumbSysmonPort", 0, NULL, 0, NULL, &port);

	if (FAILED(hr)) {
		std::cout << "Failed to connect to the port, Error: " << std::hex << hr << std::endl;
		return -1;
	}

	std::cout << "Connected to DumbSysmon! Waiting for events..." << std::endl;
	struct {
		FILTER_MESSAGE_HEADER Header;
		SYSMON_EVENT Event;
	} msg;

	while (TRUE) {
		hr = FilterGetMessage(port, &msg.Header, sizeof(msg), NULL);

		if (SUCCEEDED(hr)) {
			if (msg.Event.create) {
				std::wcout << L"[+] Process Created: " << msg.Event.ImagePath
					<< L" (PID: " << (ULONG_PTR)msg.Event.pid << L")" << std::endl;
			}
			else {
				std::wcout << L"[-] Process Exited: (PID: "
					<< (ULONG_PTR)msg.Event.pid << L")" << std::endl;
			}
		}
	}

	CloseHandle(port);
	return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file

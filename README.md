# DumbSysmon
The objective of this repository is to build a sysmon-style monitor and learn the fundamentals of Windows system monitoring, kernel callbacks, ETW, minifilter drivers, and more.


## Features
- Process Creation Monitoring
- Process Termination Monitoring
- Image Load Monitoring [TODO]
- Network Connection Monitoring [TODO]
- File Creation Monitoring [TODO]
- Registry Monitoring [TODO]
- ETW Integration [TODO]
- Minifilter Driver for File Monitoring [TODO]

## Requirements
- Windows 10/11 (ARM64 supported)
- Visual Studio 2019 or later
- Windows Driver Kit (WDK)
- Administrator privileges for installation

## Build
1. Clone the repository:
   ```bash
   git clone https://github.com/IR-Collective/dumbsysmon.git
   ```
2. Open `mydumbsysmon.slnx` in Visual Studio.
3. Select the desired configuration (for example `ARM64|Debug`).
4. Build the solution.

## Installation
This is a software-only kernel driver. Install using the INF from the build output folder. Note: This is a work in progress driver and not sutied to run on base machine, use a VM or test machine.

1. Ensure test signing is enabled (or use a properly signed driver):
   ```bash
   bcdedit /set testsigning on
   ```
2. Reboot.
3. You can use `sc.exe` to load the built `.sys`:

```bash
sc create DumbSysmon type= kernel binPath= "C:\Path\To\mydumbsysmon.sys"
sc start DumbSysmon
```

## Uninstall
```bash
sc stop DumbSysmon
sc delete DumbSysmon
```


## Usage
The driver logs process creation events using `KdPrint` and can be observed with a kernel debugger or DbgView.

User-mode logging is a planned enhancement.

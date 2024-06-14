# Aero Window Manager
Aero Window Manager is a configuration utility for modifying the metrics of objects drawn by DWM frames on Windows 10. It currently supports 1809 (build 17763), 1903/09 (builds 18362 and 18363), as well as 2004-22H2 (builds 19041-19045).

## How to install

The MSVC runtime is required to run AWM. If the application doesn't launch after performing the steps below, please try installing that first.

1. Unzip the contents of the file you downloaded to `C:\awm`. This should make the path of `awmdll.dll` be `C:\awm\awmdll.dll`. **NOTE**: You can name the root folder differently, but it has to be in the root of the C:\ drive for the application to function properly.
2. Open a `cmd` process as administrator and execute the command `regsvr32 c:\awm\msdia140_awm.dll`. A message should pop up telling you that the operation has succeeded.

**NOTE**: Steps 3 and 4 can be bypassed by using Task Scheduler or any other way to launch an executable as SYSTEM.

3. Download [PsTools](https://learn.microsoft.com/en-us/sysinternals/downloads/pstools).
4. In a `cmd` process running as administrator, navigate to the folder that you unzipped PsTools into (for example `c:\pstools`) and run the following command: `psexec64 -s -i cmd.exe`. This should open a new Command Prompt window running as SYSTEM.
5. In this new Command Prompt window, type `cd c:\awm` (or the folder you installed AWM to), followed by `injector`. After the injector process has initialized and injected `awmdll.dll` into DWM, a new console window titled "dwm.exe" should open. Do not close this window, as it will restart DWM. The awmdll.dll process will download symbols from Microsoft's servers (saved to `C:\awm\symbols`), after which it will hook into different functions within DWM, modifying their behavior. Then, clicking on any window should update its appearance to match that produced by AWM. \
**NOTE**: An update that makes the "dwm.exe" window not open anymore will be pushed soon.

## Credits
Dulappy: AWM\
Valinet: libvalinet\
Neptune: Temporary GUI\
Microsoft: msdia140.dll

I might be forgetting minor contributors. If you think you've contributed to anything in the project, please let me know.

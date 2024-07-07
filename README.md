# Aero Window Manager

Aero Window Manager is a configuration utility for modifying the metrics of
objects drawn by DWM frames on Windows 10. The currently supported versions are
1809 (build 17763), 1903/09 (builds 18362 and 18363), as well as 2004-22H2
(builds 19041-19045).

## How to install

MSVC runtime 2019 or later is required to run AWM. If the application doesn't
launch after performing the steps below, please try installing that first.

1. Download the appropriate zip file for your version of Windows (If you're
   running Windows 10 Version 22H2, that would be the file labeled
   `2004-22H2.zip`). Unzip the contents of the file you downloaded to `C:\awm`.
   This should make the path of `awmdll.dll` be `C:\awm\awmdll.dll`. **NOTE**:
   You can name the root folder differently, but it has to be in the root of
   the C:\ drive to ensure the application functions properly.
2. Open a `cmd` process as administrator and execute the command `regsvr32
   c:\awm\msdia140_awm.dll`. A message should pop up telling you that the
   operation has succeeded.

**NOTE**: Steps 3 and 4 can be bypassed by using Task Scheduler or any other
way to launch an executable as SYSTEM.

3. Download
   [PsTools](https://learn.microsoft.com/en-us/sysinternals/downloads/pstools).
4. In a `cmd` process running as administrator, navigate to the folder that you
   unzipped PsTools into (for example `c:\pstools`) and run the following
   command: `psexec64 -s -i cmd.exe`. This should open a new Command Prompt
   window running as SYSTEM.
5. In this new Command Prompt window, type `cd c:\awm` (or the folder you
   installed AWM to), followed by `injector`. The awmdll.dll process will
   download symbols from Microsoft's servers (saved to `C:\awm\symbols`), after
   which it will hook into different functions within DWM, modifying their
   behavior. Then, clicking on any window should update its appearance to match
   that produced by AWM.

## How to build

You need CMake and the v142 / VS 2019 toolset (use the VS installer to make
sure) to compile; this assumes CMake uses MSVC 2022 by default.

To compile for Release|x64, open Developer Command Prompt (x64) and type these
commands:

```batch
cd libraries/funchook
mkdir build
cd build
cmake -T v142 ..
cmake --build . --config release
cd ../../..
msbuild -p:configuration=release -p:platform=x64 awm.sln
```

Now the output directory should appear, and you should be able to copy its
contents to `c:/awm`.

## Technical - how to update AWM settings during runtime

awmdll, the DLL loaded into DWM, uses [Win32's Event Objects](https://learn.microsoft.com/en-us/windows/win32/sync/using-event-objects)
to update its settings from `HKLM/Software/AWM`. The event name is
"awmsettingschanged."

Example C code to update the settings (won't work if AWM isn't already
running):

```c
#include <windows.h>
int main(){
    HANDLE hEvent = NULL;
    hEvent = OpenEvent( EVENT_MODIFY_STATE, FALSE, TEXT("awmsettingschanged") );
    if(hEvent == NULL) return GetLastError();
    SetEvent(hEvent);
    return ERROR_SUCCESS;
}
```

## Registry Keys

These reside in `HKLM/Software/AWM`.
| Key Name | Value Range | Default Value |
| --- | --- | --- |
| Window_CornerRadiusX | (0-inf) | 0 |
| Window_CornerRadiusY | (0-inf) | 0 |
| TB_ButtonAlign | (0-4) (top, center, tbcenter, bottom, full) | full |
| TB_ButtonAlignPal | (0-4) (top, center, tbcenter, bottom, full) | top |
| TB_TargetHeight | (0-inf) | 22 |
| TB_TargetHeightPal | (0-inf) | 22 |
| TB_XBtnHeight | (0-inf) | 30 |
| TB_XBtnHeightLone | (0-inf) | 30 |
| TB_XBtnHeightPal | (0-inf) | 20 |
| TB_MidBtnHeight | (0-inf) | 30 |
| TB_EdgeBtnHeight | (0-inf) | 30 |
| TB_XBtnWidth | (0-inf) | 47 |
| TB_XBtnWidthLone | (0-inf) | 34 |
| TB_XBtnWidthPal | (0-inf) | 20 |
| TB_MidBtnWidth | (0-inf) | 46 |
| TB_EdgeBtnWidth | (0-inf) | 47 |
| TB_XBtnInsTop | (-inf-inf) | 1 |
| TB_XBtnInsTopLone | (-inf-inf) | 1 |
| TB_MidBtnInsTop | (-inf-inf) | 1 |
| TB_EdgeBtnInsTop | (-inf-inf) | 1 |
| TB_XBtnInsTopMax | (-inf-inf) | 0 |
| TB_XBtnInsTopLoneMax | (-inf-inf) | 0 |
| TB_MidBtnInsTopMax | (-inf-inf) | 0 |
| TB_EdgeBtnInsTopMax | (-inf-inf) | 0 |
| TB_XBtnOffset | (-inf-inf) | -1 |
| TB_XBtnOffsetMax | (-inf-inf) | 1 |
| TB_XBtnOffsetPal | (-inf-inf) | 1 |
| TB_XBtnAfter | (0-inf) | 0 |
| TB_MidBtnAfter | (0-inf) | 0 |
| TB_EdgeBtnAfter | (0-inf) | 0 |
| TB_InsetLeftAdd | (0-inf) | 0 |
| TB_InsetLeftMul | (0-inf) | 2 |
| TB_InsetLeftAddMax | (0-inf) | 2 |
| TB_InsetLeftMulMax | (0-inf) | 1 |
| TB_TextInset | (0-inf) | 5 |
| TB_BtnInactiveOpacity | (0-100) | 40 |
| UseTransparency | (0-1) | 0 |
| AccentPolicy | (0-4) | 0 |
| Window_ColorRActive | (0-255) | 255 |
| Window_ColorGActive | (0-255) | 255 |
| Window_ColorBActive | (0-255) | 255 |
| Window_ColorRInactive | (0-255) | 255 |
| Window_ColorGInactive | (0-255) | 255 |
| Window_ColorBInactive | (0-255) | 255 |
| Window_ColorBalanceActive | (0-100) | 100 |
| Window_ColorBalanceInactive | (0-100) | 100 |
| Text_ColorRActive | (0-255) | 0 |
| Text_ColorGActive | (0-255) | 0 |
| Text_ColorBActive | (0-255) | 0 |
| Text_ColorAActive | (0-255) | 255 |
| Text_ColorRInactive | (0-255) | 0 |
| Text_ColorGInactive | (0-255) | 0 |
| Text_ColorBInactive | (0-255) | 0 |
| Text_ColorAInactive | (0-255) | 102 |
| Text_ShadowColorRActive | (0-255) | 0 |
| Text_ShadowColorGActive | (0-255) | 0 |
| Text_ShadowColorBActive | (0-255) | 0 |
| Text_ShadowColorAActive | (0-255) | 0 |
| Text_ShadowColorRInactive | (0-255) | 0 |
| Text_ShadowColorGInactive | (0-255) | 0 |
| Text_ShadowColorBInactive | (0-255) | 0 |
| Text_ShadowColorAInactive | (0-255) | 0 |
| Text_ShadowOffsetX | (-inf-inf) | 0 |
| Text_ShadowOffsetY | (-inf-inf) | 0 |
| Text_Alignment | (0-4) | left |
| TB_IconTextVertAlign | (0-1) | center |
| Text_GlowOpacityActive | (0-100) | 0 |
| Text_GlowOpacityInactive | (0-100) | 0 |

## Credits
Dulappy: AWM\
Valinet: libvalinet\
Neptune: Temporary GUI\
Microsoft: msdia140.dll

I might be forgetting minor contributors. If you think you've contributed to
anything in the project, please let me know.

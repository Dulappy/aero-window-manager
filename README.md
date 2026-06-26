# Aero Window Manager

This is a branch of Aero Window Manager, which works on Windows 7. The goal is
to create a proper codebase for everything else to be based on, as well as tinker
with Windows 7's DWM.

**NOTE:** This version of AWM is not feature complete, and may never be. This is
simply an experiment.

## How to install

MSVC runtime 2015 (v14.0) or later is required to run AWM. If the application doesn't
launch after performing the steps below, please try installing that first.

1. Compile the software using the Release (x64) branch.
2. Copy msdia140.dll from `libraries\mioslibs\miossymbols\diasdk\bin\amd64` of
   the repository, into `<install directory>\msdia140_awm.dll` (i.e., `C:\awm\
   msdia140_awm.dll`).
3. Open a `cmd` process as administrator and execute the command `regsvr32
   <install directory>\msdia140_awm.dll`. A message should pop up letting you know
   that the operation has succeeded.
4. Run the injector as a user, since DWM runs like any other user process on
   Windows 7.

## How to build

You need CMake and the v143 / VS 2022 toolset (use the VS installer to make
sure) to compile; this assumes CMake uses MSVC 2022 by default.

To compile for Release|x64, open Developer Command Prompt (x64) and type these
commands:

```batch
cd libraries/funchook
mkdir build
cd build
cmake -T v143 ..
cmake --build . --config release
cd ../../..
msbuild -p:configuration=release -p:platform=x64 awm.sln
```

Now the output directory should appear, and you should be able to copy its
contents to `C:/awm`.

## Credits
Dulappy: AWM\
Valinet: libvalinet\
Neptune: Temporary GUI\
Wily Coyote: Text antialiasing and repo cleanup\
Microsoft: msdia140.dll

I might be forgetting minor contributors. If you think you've contributed to
anything in the project, please let me know.

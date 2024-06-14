# libvalinet
A header-only collection of generic implementations shared between multiple projects.

To use in your project, simply include any of the header files. Methods beginning with "Vn" are meant to be called (are "public"), while methods internal to a certain header file are prefixed with "libvalinet_".

Headers that compile only with C++ have an ".hpp" extension. ".h" files compile with both C, and C++ compilers.

The following header files are currently available:

* hooking/exeinject.h (-) - injects a DLL into a process and starts its entry point, reinjects should a crash occur
* ini/ini.h (-) - manipulates ini files (subset of [CIni](https://www.codeproject.com/Articles/5401/CIni) ported to C)
* internet/get.h - downloads a resource from the Internet and saves it to a file (uses WinINet)
* internet/post/h - posts data to a web resource (uses WinINet)
* pdb/pdb.h (depends on internet/get.h) [WIP] - downloads the symbol file of a library (by obtaining the path from the DLL), and retrieves the addresses of a set of symbol names; this works well, but is unfinished, having non-conformant method and symbol names; it is a subset of [pdbdump](https://gist.github.com/mridgers/2968595), and [PDBDownloader](https://github.com/rajkumar-rangaraj/PDB-Downloader) (ported from C# to C)

Items marked with [WIP] are subject to having their "public" methods (structure) changed (names, parameters etc). Other items are subject to having new methods added or critical fixes implemented to existing methods.

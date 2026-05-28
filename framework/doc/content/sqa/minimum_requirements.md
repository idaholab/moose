### Minimum System Requirements

!style! halign=left
In general, the following is required for MOOSE-based development:
!style-end!

A [!ac](POSIX) compliant Unix-like operating system. This includes any modern Linux-based operating
system (e.g., Ubuntu, Fedora, Rocky, etc.), or a Macintosh machine running either of the last two
MacOS releases.

| Hardware | Information |
| :- | :- |
| CPU Architecture | x86_64, ARM (Apple Silicon) |
| Memory | 8 GB (16 GBs for debug compilation) |
| Disk Space | 30GB |

!! comment to force MooseDocs to separate the tables

| Libraries | Version / Information |
| :- | :- |
| GCC | [!package!gcc_apptainer_min] - [!package!gcc_conda] |
| LLVM/Clang | [!package!clang_conda] - [!package!clang_apptainer] |
| Python | [!package!python_all] |
| Python Packages | [!package!python_packages] |

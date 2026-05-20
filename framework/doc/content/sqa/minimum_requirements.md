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
| GCC | [!package!apptainer_gcc_min] - [!package!conda_gcc] |
| LLVM/Clang | [!package!conda_clang] - [!package!apptainer_clang] |
| Python | [!package!all_python] |
| Python Packages | [!package!python_packages] |

# Windows Subsystem for Linux

!alert! warning
Using MOOSE on Windows 10 and 11 is experimental and not fully supported.

Caveats:

- Peacock does not work correctly (artifacts during rendering: surface normals are flipped).
- Different flavors of Linux are available, but *be sure to choose a distribution which we support*.
  While MOOSE will ultimately work on just about every flavor of Linux, this document assumes you
  have chosen Ubuntu 20.04, which is the Windows Subsystem for Linux (WSL) default as of December 2022.
!alert-end!

!include installation/wsl.md

### Close the WSL terminal

!style! halign=left
With the above complete, close the WSL terminal and re-open it. Proceed to follow (or return to) any
of the Linux instructions on our
[Getting Started](getting_started/installation/index.md optional=True) page.
!style-end!

## Tips


- Like WSL installation, the following sections require performing all commands in a PowerShell or
  Command Prompt in *administrator* mode!
- Your Download's folder while using WSL is located at: `/mnt/c/Users/<Your User Name>/Downloads`

### Updating the WSL Linux Kernel

!style! halign=left
The WSL linux kernel receives periodic updates. To perform these updates, one can run:
!style-end!

```bash
wsl --update
```

### Change WSL Version

!style! halign=left
In this instruction set, WSL version 2 is used (and is the default, recommended release). If WSL version
1 is desired, this can be changed by performing the command:
!style-end!

```bash
wsl --set-version 1
```

The default for new Linux kernel instances can be set by performing the following:

```bash
wsl --set-default-version n
```

where `n` can be replaced by either 1 or 2, depending on the version desired.

### Shutdown All WSL Instances

!style! halign=left
To shutdown all instances of WSL on the machine, perform the following:
!style-end!

```bash
wsl --shutdown
```

WSL can then be restarted from the Start menu.

Begin by performing the following external instructions:

- Install [VcXsrv](https://sourceforge.net/projects/vcxsrv/reviews/) (Only needed for Peacock)

## Launch VcXsrv

Each time you reboot, or basically each time VcXsrv is *not* running, and you wish to use the graphical
capabilities of MOOSE (Peacock), you should start VcXsrv before launching your WSL terminal.

When starting VcXsrv, options for the server can be adjusted. In general, the default options are
adequate, except for the following:

- Check "Disable access control". This allows the WSL instance to open windows via VcXsrv without
  requiring an authorization protocol.

Again, all other options for VcXsrv can remain the default.

!alert tip title=VcXsrv network connections
In some cases, the system firewall may block connections to the X window server. If errors or window
rendering stalls are experienced after adopting the VcXsrv settings above, check to make sure that all
network connections to VcXsrv are allowed.

## Install WSL

!alert note
To use the simplified installation instructions outlined below, a recent build of Windows should be
used (10 version 2004+ and build 19041+, or 11). For more information see
[Microsoft's information page on WSL](https://learn.microsoft.com/en-us/windows/wsl/install).

Open PowerShell or Windows Command Prompt in *administrator* mode by right-clicking the application
and selecting "Run as administrator". In the prompt that appears, one command can be used to install
WSL version 2 dependencies and a default Linux distribution of Ubuntu 20.04 LTS.

```bash
wsl --install
```

Program output will appear showing installation and setup of dependencies before prompting the user
for a restart to finish initial setup. See below for an example of the summary output.

```bash
C:\Users\USER>wsl --install
Installing: Virtual Machine Platform
Virtual Machine Platform has been installed.
Installing: Windows Subsystem for Linux
Windows Subsystem for Linux has been installed.
Downloading: WSL Kernel
Installing: WSL Kernel
WSL Kernel has been installed.
Downloading: Ubuntu
The requested operation is successful. Changes will not be effective until the system is rebooted.
```

### Setup Linux and Enable GUI Dependencies

Once the restart is complete, a prompt will appear on next boot continuing the Ubuntu Linux
installation and requesting a username for the new UNIX user account.

```
Installing, this may take a few minutes....
Please create a default UNIX user account. The username does not need to match your Windows username.
For more information visit: https://aka.ms/wslusers
Enter new UNIX username:
```

After entering a username and a new password, a standard Linux bash prompt will appear. An update
should be performed to check for any out-of-date packages. To enable usage of Peacock, OpenGL libraries
must also be installed.

```bash
sudo apt update
sudo apt upgrade
sudo apt install x11-apps libglu1-mesa
```

### Configure WSL to use VcXsrv

Modify the bash profile to allow WSL to connect to VcXsrv.

```bash
echo "export DISPLAY=$(cat /etc/resolv.conf | grep nameserver | awk '{print $2}'):0.0" >> ~/.bashrc
```

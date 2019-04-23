# Windows 10

!alert! warning
Using MOOSE on Windows 10 is experimental and not fully supported.

Caveats:

- Peacock does not work correctly (artifacts during rendering: surface normals are flipped).
- Different flavors of Linux are available.

    - Be sure to choose an OS in which we support. While MOOSE will ultimately work on just about every flavor of Linux, this document assumes you have chosen Ubuntu.
!alert-end!

Begin by performing the following external sets of instructions. Remember to choose Ubuntu, unless you know what you are doing.

- [Windows Subsystem for Linux (WSL)](https://msdn.microsoft.com/en-us/commandline/wsl/install_guide)
- [VcXsrv](https://sourceforge.net/projects/vcxsrv/reviews/) (Only needed for Peacock)

## Launch VcXsrv

Each time you reboot, or basically each time VcXsrv is *not* running, and you wish to use the graphical capabilities of MOOSE (Peacock), you should start VcXsrv before launching your WSL terminal.

We have found better performance instructing VcXsrv to use software rendering over native OpenGL. When launching VcXsrv, search for the option: "Native opengl", and un-check it. All other options can remain the default.

## Edit Hostname within WSL

Launch the Windows 10 Command Prompt as an Administrator (right-click the Windows Start button, and select Command Prompt(Admin) in the resulting menu that appears), and modify the Windows 10 hosts file (located in `C:\Windows\System32\Drivers\etc\hosts`) to include the results of `hostname` to resolve to 127.0.0.1. This is necessary due to the way
MPICH (a message passing interface) passes information among itself when running applications (like MOOSE) in parallel.

```
C:\Users\[your_user_name]> hostname
DESKTOP-L7BGA7L

C:\Users\[your_user_name]> Notepad "C:\Windows\System32\Drivers\etc\hosts"
# localhost name resolution is handled within DNS itself.
#       127.0.0.1       localhost
#       ::1             localhost

127.0.0.1   DESKTOP-L7BGA7L    <---- ADD THAT
```

## Update Ubuntu, install additional libraries

Launch WSL, perform an update, and install necessary OpenGL libraries:

```bash
sudo apt-get update
sudo apt-get upgrade
sudo apt-get install x11-apps libglu1-mesa
```

## Configure WSL to use VcXsrv

Modify your bash profile to allow WSL to connect to VcXsrv.

```bash
echo "export DISPLAY=localhost:0" >> ~/.bashrc
```

## Close the WSL terminal

With the above complete, close the WSL terminal, and re-open it. Then proceed to our [Ubuntu](getting_started/installation/ubuntu.md) instructions.

!alert! note
Your Download's folder, while using WSL, is located at: `/mnt/c/Users/<Your User Name>/Downloads`
!alert-end!

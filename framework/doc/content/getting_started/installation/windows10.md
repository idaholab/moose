# Windows 10

!alert! warning
Using MOOSE on Windows 10 is experimental and not fully supported.

Known issues:

- Peacock does not work correctly.
- Microsoft is now offering several different flavors of linux. Keep in mind, the Microsoft Store may not contain the same version of said operating system that we provide a support package for. Do some investigating to make sure the operating system (and version) you choose, is among our list of supported linux platforms. In general, we have support packages for the following linux operating systems listed below at their latest available release. At the time of this writing, the safe bet, is to choose Ubuntu:

  - Ubuntu
  - Mint
  - CentOS
  - OpenSUSE
  - Fedora
!alert-end!

Begin by performing the following external sets of instructions. Do note, that when it comes time to decide what distribution of Linux you want, you should try and choose a flavor listed among our getting started pages for the smoothest installation process. This is not a requirement however, as MOOSE will run on just about any flavor of Linux if you are of the adventurous type!

- [Windows Subsystem for Linux (WSL)](https://msdn.microsoft.com/en-us/commandline/wsl/install_guide)
- [VcXsrv](https://sourceforge.net/projects/vcxsrv/reviews/) (Only needed for Peacock)

## Edit Hostname within WSL

Launch WSL, and modify the /etc/hosts file to include the results of `hostname` to resolve to 127.0.0.1. This is necessary due to the way
MPICH (a message passing interface) passes information among itself when running applications (like MOOSE) in parallel.

```bash
[~]> hostname
DESKTOP-L7BGA7L

[~]> sudo vi /etc/hosts
127.0.0.1   localhost
127.0.0.1   DESKTOP-L7BGA7L    <---- ADD THAT
```

## Update Ubuntu/OpenSUSE within WSL

Launch WSL, and perform an update:

```bash
# Ubuntu:
sudo apt-get update
sudo apt-get upgrade

# OpenSUSE:
sudo zypper update
```

## Setup Xming

Launch WSL, and modify your bash profile to allow WSL to connect to Xming.

```bash
echo "export DISPLAY=localhost:0" >> ~/.bashrc
```

Once the above items are complete, please follow the installation instructions pertaining to your choice of Linux OS among our getting started pages. If you chose a Linux distribution we do not have a package for, then you will want to follow one of our 'Manual Install' instructions.

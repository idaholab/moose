# Windows 10

The Windows 10 installation is experimental and not fully supported. To begin install the
following.

* [Windows Subsystem for Linux (WSL)](https://msdn.microsoft.com/en-us/commandline/wsl/install_guide)
* [Xming](https://sourceforge.net/projects/xming/)

After these two items are installed the following steps are required to complete the
setup for the WSL to work for MOOSE. After these steps are completed then follow the install instructions for [Ubuntu](installation/ubuntu.md), but be sure to
restart the bash session after installing the redistributable package.


##### (1) Edit Hostname

Modify the /etc/hosts file and add the results of `hostname` to resolve to 127.0.0.1 as follows.

```bash
[~]> hostname
DESKTOP-L7BGA7L
[~]> sudo vi /etc/hosts
127.0.0.1   localhost
127.0.0.1   DESKTOP-L7BGA7L    <---- ADD THAT
```

##### (2) Update Ubuntu on WSL
It is also required to update Ubuntu to latest version.

```bash
sudo apt-get update
sudo apt-get upgrade
```

##### (3) Setup Xming
Allow your Bash session to connect to Xming.

```bash
echo "export DISPLAY=:0" >> ~/.bashrc
```

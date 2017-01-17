# Windows 10

{!docs/content/getting_started/minimum_requirements.md!}

---
## Pre-Reqs
* [Enable Windows Subsystem for Linux](https://msdn.microsoft.com/en-us/commandline/wsl/install_guide)
* Install [Xming](https://sourceforge.net/projects/xming/)

---
## Run Bash
* modify the /etc/hosts file and add the results of `hostname` to resolve to 127.0.0.1

```bash
[~]> hostname
DESKTOP-L7BGA7L
[~]> sudo vi /etc/hosts
127.0.0.1   localhost
127.0.0.1   DESKTOP-L7BGA7L    <---- ADD THAT
```

* Update Ubuntu to latest version

```bash
sudo apt-get update
sudo apt-get upgrade
```

* Install the Ubuntu pre-reqs for MOOSE development


```bash
  sudo apt-get install build-essential \
gfortran \
tcl \
git \
m4 \
freeglut3 \
doxygen \
libblas-dev \
liblapack-dev \
libx11-dev \
libnuma-dev \
libcurl4-gnutls-dev \
zlib1g-dev \
libhwloc-dev
x11-apps \
libglu1-mesa
```


* Download our redistributatble package for Ubuntu 14.04
    * !MOOSEPACKAGE arch=ubuntu14.04 return=link!

{!docs/content/getting_started/installation/install_redistributable_deb.md!}

* Allow your Bash session to connect to Xming

    <pre>
    echo "export DISPLAY=:0" >> ~/.bashrc
    </pre>

* Restart Bash session (so the moose-environment gets loaded)

{!docs/content/getting_started/installation/clone_moose.md!}
{!docs/content/getting_started/installation/build_libmesh.md!}
{!docs/content/getting_started/installation/conclusion.md!}

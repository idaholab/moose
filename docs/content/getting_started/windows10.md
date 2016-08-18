# Windows 10

{!content/getting_started/minimum_requirements.md!}

---
## Pre-Reqs
* [Enable Windows Subsystem for Linux](https://msdn.microsoft.com/en-us/commandline/wsl/install_guide)
* Install [Xming](https://sourceforge.net/projects/xming/)

---
## Run Bash
* modify the /etc/hosts file and add the results of `hostname` to resolve to 127.0.0.1

    <pre>
    [~]> hostname
    DESKTOP-L7BGA7L
    [~]> sudo vi /etc/hosts
    127.0.0.1   localhost
    127.0.0.1   DESKTOP-L7BGA7L    <---- ADD THAT
    </pre>

* Update Ubuntu to latest version

    <pre>
    sudo apt-get update
    sudo apt-get upgrade
    </pre>

* Install the Ubuntu pre-reqs for MOOSE development


    <pre>
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
    </pre>


* Download our redistributatble package for Ubuntu 14.04
    * !MOOSEPACKAGE arch=ubuntu14.04 return=link!

{!content/getting_started/install_redistributable_deb.md!}

* Allow your Bash session to connect to Xming

    <pre>
    echo "export DISPLAY=:0" >> ~/.bashrc
    </pre>

* Restart Bash session (so the moose-environment gets loaded)

{!content/getting_started/clone_moose.md!}
{!content/getting_started/build_libmesh.md!}
{!content/getting_started/conclusion.md!}
# Ubuntu

!include docs/content/getting_started/minimum_requirements.md

---
## Pre-Reqs
* Install the following using apt-get

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
```

* Download one of our redistributables according to your version of Ubuntu

    * Ubuntu 16.04: !moosepackage arch=ubuntu16.04 return=link!
    * Ubuntu 14.04: !moosepackage arch=ubuntu14.04 return=link!

!include docs/content/getting_started/installation/install_redistributable_deb.md
!include docs/content/getting_started/installation/clone_moose.md
!include docs/content/getting_started/installation/build_libmesh.md
!include docs/content/getting_started/installation/conclusion.md

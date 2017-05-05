# Mint

!include docs/content/getting_started/minimum_requirements.md

---
## Pre-Reqs
* Install the following using apt-get

```bash
  sudo -E apt-get install build-essential \
gfortran \
tcl \
freeglut3 \
libX11-dev \
libblas-dev \
liblapack-dev \
git \
m4
```

* Download one our redistributables according to your version of Mint

    * Mint 18: !moosepackage arch=mint18 return=link!

!include docs/content/getting_started/installation/install_redistributable_deb.md
!include docs/content/getting_started/installation/clone_moose.md
!include docs/content/getting_started/installation/build_libmesh.md
!include docs/content/getting_started/installation/conclusion.md

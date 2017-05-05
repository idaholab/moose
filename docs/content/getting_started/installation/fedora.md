# Fedora

!include docs/content/getting_started/minimum_requirements.md

---
## Pre-Reqs
* Install the following using dnf

```bash
  sudo -E dnf install gcc \
gcc-c++ \
gcc-gfortran \
tcl \
tk \
findutils \
make \
freeglut-devel \
libXt-devel \
libX11-devel \
m4 \
blas-devel \
lapack-devel \
git
```

* Download one our redistributables according to your version of Fedora

    * Fedora 25: !moosepackage arch=fedora25 return=link!

!include docs/content/getting_started/installation/install_redistributable_rpm.md
!include docs/content/getting_started/installation/clone_moose.md
!include docs/content/getting_started/installation/build_libmesh.md
!include docs/content/getting_started/installation/conclusion.md

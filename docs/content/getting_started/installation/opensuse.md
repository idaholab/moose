# OpenSUSE

!include docs/content/getting_started/minimum_requirements.md

---
## Pre-Reqs
* Install the following using zypper

```bash
  sudo -E zypper install gcc-c++ \
gcc-fortran \
make \
libX11-devel \
blas-devel \
lapack-devel \
freeglut-devel \
m4 \
git
```

* Download one our redistributables according to your version of OpenSUSE

    * OpenSuSE Leap: !moosepackage arch=opensuseleap return=link!

!include docs/content/getting_started/installation/install_redistributable_rpm.md
!include docs/content/getting_started/installation/clone_moose.md
!include docs/content/getting_started/installation/build_libmesh.md
!include docs/content/getting_started/installation/conclusion.md

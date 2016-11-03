# OpenSUSE

{!content/getting_started/minimum_requirements.md!}

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

    * OpenSuSE 42.1: !MOOSEPACKAGE arch=opensuse42.1 return=link!

{!content/getting_started/install_redistributable_rpm.md!}
{!content/getting_started/clone_moose.md!}
{!content/getting_started/build_libmesh.md!}
{!content/getting_started/conclusion.md!}
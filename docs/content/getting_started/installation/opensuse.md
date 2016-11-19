# OpenSUSE

{!docs/content/getting_started/minimum_requirements.md!}

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

{!docs/content/getting_started/installation/install_redistributable_rpm.md!}
{!docs/content/getting_started/installation/clone_moose.md!}
{!docs/content/getting_started/installation/build_libmesh.md!}
{!docs/content/getting_started/installation/conclusion.md!}

# CentOS

!include docs/content/getting_started/minimum_requirements.md

---
## Pre-Reqs
* Install the following using yum

```bash
  sudo -E yum install gcc \
gcc-c++ \
make \
freeglut-devel\
m4 \
blas-devel \
lapack-devel \
gcc-gfortran \
libX11-devel
```

* Download one our redistributables according to your version of CentOS

    * CentOS 7: !moosepackage arch=centos7 return=link!

!include docs/content/getting_started/installation/install_redistributable_rpm.md
!include docs/content/getting_started/installation/clone_moose.md
!include docs/content/getting_started/installation/build_libmesh.md
!include docs/content/getting_started/installation/conclusion.md

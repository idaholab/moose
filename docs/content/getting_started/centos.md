# CentOS

{!docs/content/getting_started/minimum_requirements.md!}

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

    * CentOS 7: !MOOSEPACKAGE arch=centos7 return=link!

{!docs/content/getting_started/install_redistributable_rpm.md!}
{!docs/content/getting_started/clone_moose.md!}
{!docs/content/getting_started/build_libmesh.md!}
{!docs/content/getting_started/conclusion.md!}

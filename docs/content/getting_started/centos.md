# CentOS

{!content/getting_started/minimum_requirements.md!}

---
## Pre-Reqs
* Install the following using yum

    <pre>
    sudo -E yum install gcc \
    gcc-c++ \
    make \
    freeglut-devel\
    m4 \
    blas-devel \
    lapack-devel \
    gcc-gfortran \
    libX11-devel
    </pre>

* Download one our redistributables according to your version of CentOS

    * CentOS 7: !MOOSEPACKAGE arch=centos7 return=link!

{!content/getting_started/install_redistributable_rpm.md!}
{!content/getting_started/clone_moose.md!}
{!content/getting_started/build_libmesh.md!}
{!content/getting_started/conclusion.md!}
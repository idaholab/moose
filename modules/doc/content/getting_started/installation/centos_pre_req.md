## Prerequisites

Install the following using yum

```bash
sudo -E yum install gcc \
  gcc-c++ \
  make \
  freeglut-devel\
  m4 \
  blas-devel \
  lapack-devel \
  gcc-gfortran \
  tcl-devel \
  libX11-devel \
  git \
  zlib-devel
```

Download and install one our redistributable packages according to your version of CentOS

- CentOS 7: [!package!name arch=centos7]

Once downloaded, the package can be installed via the rpm utility:

```bash
sudo rpm -i moose-environment_centos-*.rpm
```

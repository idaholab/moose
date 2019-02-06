## Prerequisites

Install the following using zypper

```bash
sudo -E zypper install gcc-c++ \
  gcc-fortran \
  make \
  libX11-devel \
  blas-devel \
  lapack-devel \
  freeglut-devel \
  tcl-devel \
  m4 \
  git
```

Download and install one our redistributable packages according to your version of OpenSUSE.

- OpenSuSE: !!package name arch=opensuse15!!

Once downloaded, the package can be installed via the rpm utility:

```bash
sudo rpm -i moose-environment_opensuse-*.rpm
```

## Prerequisites

Install the following using apt-get

```bash
sudo apt-get install build-essential \
  gfortran \
  tcl \
  git \
  m4 \
  freeglut3 \
  doxygen \
  libblas-dev \
  liblapack-dev \
  libx11-dev \
  libnuma-dev \
  libcurl4-gnutls-dev \
  zlib1g-dev \
  libhwloc-dev \
  libxml2-dev \
  libpng-dev \
  pkg-config \
  liblzma-dev
```

Download and install one of our redistributable packages according to your version of Mint. Mint is a spinoff of Ubuntu. But the versions do not necessarly match (each Mint release is based on Ubuntu's LTS release schedule): [https://en.wikipedia.org/wiki/Linux_Mint_version_history](https://en.wikipedia.org/wiki/Linux_Mint_version_history).

- Mint 19: [!package!name arch=ubuntu18]
- Mint 18: [!package!name arch=ubuntu16]

Once downloaded, the package can be installed via the dpkg utility:

```bash
sudo dpkg -i moose-environment_ubuntu-*.deb
```

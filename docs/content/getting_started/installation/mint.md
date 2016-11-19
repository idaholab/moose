# Mint

{!docs/content/getting_started/minimum_requirements.md!}

---
## Pre-Reqs
* Install the following using apt-get

```bash
  sudo -E apt-get install build-essential \
gfortran \
tcl \
freeglut3 \
libX11-dev \
libblas-dev \
liblapack-dev \
git \
m4
```

* Download one our redistributables according to your version of Mint

    * Mint 17.3: !MOOSEPACKAGE arch=mint17.3 return=link!
    * Mint 17.1: !MOOSEPACKAGE arch=mint17.1 return=link!

{!docs/content/getting_started/installation/install_redistributable_deb.md!}
{!docs/content/getting_started/installation/clone_moose.md!}
{!docs/content/getting_started/installation/build_libmesh.md!}
{!docs/content/getting_started/installation/conclusion.md!}

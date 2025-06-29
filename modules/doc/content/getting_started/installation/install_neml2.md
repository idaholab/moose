# NEML2

In addition to the MOOSE native material models, MOOSE can also seamlessly interface with the external material modeling library [NEML2](https://github.com/applied-material-modeling/neml2) [!cite](neml2osti,osti_1961125).

NEML2, the New Engineering Material modeling Library, version 2, is an offshoot of [NEML](https://github.com/Argonne-National-Laboratory/neml), an earlier material modeling code developed at Argonne National Laboratory. the library is provided as open source software under a MIT license.

NEML2 extends the key philosophy of its predecessor, i.e., material models are flexible, modular, and can be built from smaller blocks. It also provides modern features that do not exist in the framework of its predecessor such as material model vectorization, automatic differentiation, device-portable just-in-time compilation, operator fusion, lazy tensor evaluation, etc. Moreover, NEML2 can seamlessly integrate with the popular machine learning package PyTorch to take advantage of modern and fast-growing machine learning techniques.

## Installation

NEML2 depends on libtorch. See the [libtorch installation guide](getting_started/installation/install_libtorch.md optional=True) for instructions on obtaining libtorch.

!alert! tip
If libtorch was downloaded/installed to a non-default location, it is a good idea to set the environment variable `LIBTORCH_DIR` to make sure the same libtorch installation is consistently used throughout the build process.

```bash
export LIBTORCH=/path/to/libtorch
```

!alert-end!

To install NEML2, simply run the following script

```bash
cd ~/projects/moose
./scripts/update_and_rebuild_neml2.sh
```

!alert tip
The setup script uses sensible defaults that work out-of-the-box. The script is also extensively customizable. Use the `--help` argument to print out a detailed help message.

Once NEML2 is successfully installed, you can configure MOOSE to use NEML2 by

```bash
./configure --with-neml2 --with-libtorch
```

!alert! tip
The `--with-neml2` configure option accepts an optional path argument, which could be useful if NEML2 was installed to a non-default location, i.e.

```bash
./configure --with-neml2=/path/to/neml2 --with-libtorch
```

!alert-end!

After that, you can follow the [getting started](getting_started/installation/index_content.md optional=True) instructions to build MOOSE as usual. The `make check_neml2` command can be used to check whether NEML2 is successfully enabled within MOOSE and inspect additional compile/link flags.

## Using NEML2 in a MOOSE simulation

A dedicated input file syntax block is reserved for MOOSE-NEML2 interaction. The entry point to the syntax block is `[NEML2]`.
Please refer to the [NEML2 syntax](syntax/NEML2/index.md) documentation for more details.

## Citing NEML2

```text
@techreport{neml2osti,
  author      = {Tianchen Hu and Mark C.  Messner and Daniel Schwen and Lynn B.  Munday and Dewen Yushu},
  title       = {NEML2: A High Performance Library for Constitutive Modeling},
  institution = {Argonne National Laboratory (ANL), Argonne, IL (United States); Idaho National Laboratory (INL), Idaho Falls, ID (United States)},
  doi         = {10.2172/2440430},
  url         = {https://www.osti.gov/biblio/2440430},
  place       = {United States},
  year        = {2024},
  month       = {09}
}

@misc{osti_1961125,
  author = {MESSNER, MARK and HU, TIANCHEN and US DOE NE-NEAMS},
  title  = {NEML2 - THE NEW ENGINEERING MATERIAL MODEL LIBRARY, VERSION 2},
  doi    = {10.11578/dc.20230314.1},
  url    = {https://www.osti.gov/biblio/1961125},
  place  = {United States},
  year   = {2023},
  month  = {01}
}
```

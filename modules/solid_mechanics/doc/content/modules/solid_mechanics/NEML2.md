# NEML2

In addition to the MOOSE native material models, MOOSE can also seamlessly interface with the external material modeling library [NEML2](https://github.com/applied-material-modeling/neml2) [!cite](neml2_anl_report).

NEML2, the New Engineering Material modeling Library, version 2, is an offshoot of [NEML2](https://github.com/Argonne-National-Laboratory/neml), an earlier material modeling code developed at Argonne National Laboratory. the library is provided as open source software under a MIT license.

NEML2 extends the key philosophy of its predecessor, i.e., material models are flexible, modular, and can be built from smaller blocks. It also provides modern features that do not exist in the framework of its predecessor such as material model vectorization, automatic differentiation, device-portable just-in-time compilation, operator fusion, lazy tensor evaluation, etc. Moreover, NEML2 can seamlessly integrate with the popular machine learning package PyTorch to take advantage of modern and fast-growing machine learning techniques.

## Installation

NEML2 depends on libtorch. See the [libtorch installation guide](getting_started/installation/install_libtorch.md optional=True) for instructions on obtaining libtorch.

To install NEML2, simply run the following script

```bash
cd ~/projects/moose
./scripts/update_and_rebuild_neml2.sh
```

!alert tip
The setup script uses sensible defaults that work out-of-the-box. The script is also extensively customizable. Use the `--help` argument to print out a detailed help message.

After that, you can follow the [getting started](getting_started/installation/index_content.md optional=True) instructions to build MOOSE as usual.

## Using NEML2 in a MOOSE simulation

A dedicated input file syntax block is reserved for MOOSE-NEML2 interaction. The entry point to the syntax block is `[NEML2]`.
Please refer to the [NEML2 syntax](syntax/NEML2/index.md) documentation for more details.

## NEML2 MOOSE materials

A list of custom NEML2 material models that reside in MOOSE:

- [Los Alamos Reduced Order Model (LAROMance)](solid_mechanics/common/LAROMANCE6DInterpolation.md)
- [LibtorchModel](solid_mechanics/common/LibtorchModel.md)

## Citing NEML2

```text
@misc{osti_1961125,
title = {NEML2 - THE NEW ENGINEERING MATERIAL MODEL LIBRARY, VERSION 2},
author = {MESSNER, MARK and HU, TIANCHEN and US DOE NE-NEAMS},
url = {https://www.osti.gov//servlets/purl/1961125},
doi = {10.11578/dc.20230314.1},
url = {https://www.osti.gov/biblio/1961125}, year = {2023},
month = {1},
}
```

# NEML2

In addition to the MOOSE native material models, MOOSE can also seamlessly interface with the external material modeling library [NEML2](https://github.com/reverendbedford/neml2) [!cite](neml2_anl_report).

## What is NEML2?

NEML2 stands for the New Engineering Material model Library, version 2.

NEML2 is an offshoot of [NEML](https://github.com/Argonne-National-Laboratory/neml),
an earlier constitutive modeling code developed at Argonne National Laboratory (which
can be used through the MOOSE based [BlackBear](https://github.com/idaholab/blackbear) application).
Like NEML, NEML2 provides a flexible, modular way to build constitutive models from smaller blocks.
Unlike NEML, NEML2 vectorizes the constitutive update to efficiently run on GPUs.  NEML2 is
built on top of [libTorch](https://pytorch.org/cppdocs/) to provide GPU support, but this also
means that NEML2 models have all the features of a Torch module.  So, for example, users can take
derivatives of the model with respect to parameters using pytorch automatic differentiation (AD).

NEML2 is provided as open source software under a MIT [license](https://raw.githubusercontent.com/reverendbedford/neml2/main/LICENSE).

## How to use NEML2?

To enable NEML2 in MOOSE, set the environment variable `NEML2_DIR` to the path to the NEML2 directory.
Alternatively, a compatible version of NEML2 also comes with MOOSE as an optional submodule. To obtain the submodule, run

```bash
git submodule update --checkout --init --recursive modules/solid_mechanics/contrib/neml2
```

Once the environment variable `NEML2_DIR` is set to a valid path or the submodule is initialized,
you need to configure MOOSE to use libTorch (because NEML2 depends on libTorch). To do so, run

```bash
cd ~/projects/moose
./scripts/setup_libtorch.sh
./configure --with-libtorch
```

For additional details consult [our libTorch installation guide](getting_started/installation/install_libtorch.md optional=True).
Note that if you want to use a custom version of libTorch, you can point the configure script to a specific libTorch installation, i.e.

```bash
cd ~/projects/moose
./configure --with-libtorch=/path/to/libTorch
```

After that, you can follow the [getting started](getting_started/installation/index_content.md optional=True) instructions to build MOOSE as usual.

## Input file syntax

A dedicated input file syntax block is reserved for MOOSE-NEML2 interaction. The entry point to the syntax block is `[NEML2]`. An example syntax is shown below:

```python
[NEML2]
  input = 'neml2.i'
  model = 'model'
  temperature = 'T'
  verbose = true
  mode = ELEMENT
[]
```

The field `input` informs MOOSE where to look for the NEML2 input file. The field `model` tells MOOSE which material model in the NEML2 input file should be "imported". Details about all the options can be found in the [MOOSE-NEML2 syntax documentation](syntax/NEML2/index.md). To understand how to write a NEML2 input file, please refer to the [NEML2 documentation](https://reverendbedford.github.io/neml2/).

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

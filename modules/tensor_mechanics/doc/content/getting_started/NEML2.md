# NEML2

BlackBear offers a wide variety of material models. In addition to the native material models, BlackBear can also seamlessly interface with an external material modeling library called [NEML2](https://github.com/reverendbedford/neml2).

## What is NEML2?

NEML2 stands for the New Engineering Material model Library, version 2.

NEML2 is an offshoot of [NEML](https://github.com/Argonne-National-Laboratory/neml), an earlier constitutive modeling code developed at Argonne National Laboratory.
Like NEML, NEML2 provides a flexible, modular way to build constitutive models from smaller blocks.
Unlike NEML, NEML2 vectorizes the constitutive update to efficiently run on GPUs.  NEML2 is built on top of [libTorch](https://pytorch.org/cppdocs/) to provide GPU support, but this also means that NEML2 models have all the features of a Torch module.  So, for example, users can take derivatives of the model with respect to parameters using pytorch AD.

NEML2 is provided as open source software under a MIT [license](https://raw.githubusercontent.com/reverendbedford/neml2/main/LICENSE).

## How to use NEML2?

To enable NEML2 in BlackBear, set the environment variable `NEML2_DIR` to the path to the NEML2 directory.
Alternatively, a compatible version of NEML2 also comes with BlackBear as a submodule. To obtain the submodule, run

```bash
git submodule update --init --recursive contrib/neml2
```

Once the environment variable `NEML2_DIR` is set to a valid path or the submodule is initialized, you need to configure MOOSE to use libTorch (because NEML2 depends on libTorch). To do so, run

```bash
cd ~/projects/moose
./scripts/setup_libtorch.sh
./configure --with-libtorch
```

Note that if you want to use a custom version of libTorch, you can point the configure script to a specific libTorch installation, i.e.

```bash
cd ~/projects/moose
./configure --with-libtorch=/path/to/libTorch
```

After that, you can follow the [getting started](RunningBlackBear.md) instructions to build BlackBear as usual.

## Input file syntax

A dedicated input file syntax block is reserved for BlackBear-NEML2 interaction. The entry point to the syntax block is `[NEML2]`. An example syntax is shown below:

```python
[NEML2]
  input = 'neml2.i'
  model = 'model'
  temperature = 'T'
  verbose = true
  mode = ELEMENT
[]
```

The field `input` informs BlackBear where to look for the NEML2 input file. The field `model` tells BlackBear which material model in the NEML2 input file should be "imported". Details about all the options can be found in the [BlackBear-NEML2 syntax documentation](syntax/NEML2/index.md). To understand how to write a NEML2 input file, please refer to the [NEML2 documentation](https://reverendbedford.github.io/neml2/).

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

# NEML2

In addition to the MOOSE native material models, MOOSE can also seamlessly interface with the external material modeling library [NEML2](https://github.com/applied-material-modeling/neml2) [!cite](neml2softwarex,neml2osti,osti_1961125).

NEML2, the New Engineering Material modeling Library, version 2, is an offshoot of [NEML](https://github.com/Argonne-National-Laboratory/neml), an earlier material modeling code developed at Argonne National Laboratory. the library is provided as open source software under a MIT license.

NEML2 extends the key philosophy of its predecessor, i.e., material models are flexible, modular, and can be built from smaller blocks. It also provides modern features that do not exist in the framework of its predecessor such as material model vectorization, automatic differentiation, device-portable just-in-time compilation, operator fusion, lazy tensor evaluation, etc. Moreover, NEML2 can seamlessly integrate with the popular machine learning package PyTorch to take advantage of modern and fast-growing machine learning techniques.

## Installation

NEML2 is built from source and installed as a Python package that lives in the *same Python
environment as PyTorch* (a conda environment or a virtual environment). A single install provides
both the C++ libraries MOOSE links against and the Python tooling (e.g. `neml2-compile`, used by
the ahead-of-time compilation runtime). The workflow is: provide PyTorch, build and install NEML2,
then configure MOOSE.

!alert! note title=NEML2 uses the PyTorch in your Python environment
The blessed way to enable NEML2 is a PyTorch installed in the active Python environment (a pip
wheel, or a from-source build installed into that environment). This is distinct from MOOSE's
standalone [libtorch](install_libtorch.md optional=True) capability, which downloads a prebuilt
C++ libtorch distribution for MOOSE's own machine-learning features -- that download route is *not*
used to enable NEML2. The `--with-libtorch` path given at configure time (below) simply points at
the C++ libraries that ship inside your environment's PyTorch.
!alert-end!

### 1. Provide PyTorch

NEML2 links against the PyTorch found in your active Python environment. Activate that environment
first (so `python3` and `pip` resolve to it), then make PyTorch available.

**Recommended: install a PyTorch wheel with pip.** Choose a version compatible with NEML2 and matching your CUDA toolkit:

```bash
pip install torch
```

See [PyTorch download instructions](https://pytorch.org/get-started/locally/) for more details.

**Alternative: build PyTorch from source.** Only necessary when a prebuilt wheel does not fit your
needs — for example a specific CUDA architecture, a custom BLAS, or an unsupported platform. Follow
the [PyTorch from-source instructions](https://github.com/pytorch/pytorch#from-source) and install
the result into the active environment (so `python3 -c 'import torch'` resolves to it); NEML2 then
links against it exactly as it would a wheel. Building against the same OpenBLAS that MOOSE's PETSc
uses also avoids the BLAS/LAPACK conflict described below.

!alert! warning title=BLAS/LAPACK ABI conflict between the PyTorch wheel and PETSc
PyTorch wheels bundle their own BLAS/LAPACK inside `libtorch_cpu.so` (it exports the standard LP64
symbols `dgemm_`, `dgeev_`, ... as well as ILP64 `*_64_` variants). MOOSE's PETSc links its own
OpenBLAS (e.g., `petsc/arch-moose/lib/libopenblas.so.0`), which exports the *same* LP64 symbol names. When
a single MOOSE executable loads both libraries, the dynamic linker can interpose one library's
BLAS/LAPACK onto the other's calls.

Is this harmful? It is configuration-dependent. Both are standard LP64 BLAS, so in many environments
everything runs correctly (the bundled MOOSE-NEML2 test suite passes with no workaround). But if the
two implementations disagree on an ABI detail — integer width, threading, or corner-case numerics —
the interposition can produce *wrong results or crashes* in linear algebra, coming from either PETSc
solves or PyTorch operators, and often silently.

If you observe such problems, force a single, consistent BLAS/LAPACK across the process with
`LD_PRELOAD`, preloading the OpenBLAS that MOOSE/PETSc is built against so that PyTorch uses it too:

```bash
export LD_PRELOAD=$HOME/projects/moose/petsc/arch-moose/lib/libopenblas.so.0
```

This is safe because PyTorch works with any standard LP64 BLAS, while PETSc requires the specific
OpenBLAS it was built with. Building PyTorch from source against the same OpenBLAS avoids the
conflict entirely.
!alert-end!

### 2. Build and install NEML2

With PyTorch available in the active environment, run:

```bash
cd ~/projects/moose
./scripts/update_and_rebuild_neml2.sh
```

The script updates the NEML2 submodule, builds it from source, and installs it (non-editable) into
the active environment's site-packages, next to PyTorch. It *checks* — but never installs — its
prerequisites (an importable PyTorch and `cmake`), leaves your pinned PyTorch untouched, and on
success prints the exact `./configure` command to run next.

!alert tip
The script is customizable. Use the `--help` argument to print a detailed help message listing all
options and influential environment variables.

### 3. Configure MOOSE

```bash
./configure --with-neml2 --with-libtorch=$(python3 -c 'import torch, os; print(os.path.dirname(torch.__file__))')
```

Given without a path, `--with-neml2` automatically locates the NEML2 installed in the active Python
environment. To override, pass an explicit path or set the `NEML2_DIR` environment variable:

```bash
./configure --with-neml2=/path/to/neml2 --with-libtorch=/path/to/torch
```

After that, follow the [getting started](getting_started/installation/index_content.md optional=True)
instructions to build MOOSE as usual. The `make check_neml2` command can be used to check whether
NEML2 is successfully enabled within MOOSE and to inspect the additional compile/link flags.

## Using NEML2 in a MOOSE simulation

A dedicated input file syntax block is reserved for MOOSE-NEML2 interaction. The entry point to the syntax block is `[NEML2]`.
Please refer to the [NEML2 syntax](syntax/NEML2/index.md) documentation for more details.

## Citing NEML2

```text
@article{neml2softwarex,
  title = {NEML2: An efficient and modular multiphysics constitutive modeling library for hybrid computing environments},
  journal = {SoftwareX},
  volume = {31},
  pages = {102302},
  year = {2025},
  issn = {2352-7110},
  doi = {https://doi.org/10.1016/j.softx.2025.102302},
  url = {https://www.sciencedirect.com/science/article/pii/S2352711025002687},
  author = {Tianchen Hu and Mark C. Messner},
  keywords = {Constitutive model, GPU, Multiphysics}
}
```

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
```

```text
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

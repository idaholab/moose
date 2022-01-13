# Install MOOSE with Pytorch C++ API (libtorch)

The way one enables pytorch capabilities in MOOSE depends on
the operating system (Linux or Mac) and if we use HPC or just a local workstation.

!alert! note
Before we review the main approaches, it is important to emphasize that
linking MOOSE with libtorch on Linux machines is not support if the compiler stack has been built
using a `libc` version below 2.27 (for `libtorch v 1.8+`)
or 2.23 (for `libtorch v1.4-1.8`). Furthermore, we do not support `libtorch` versions below
v1.4. To check your currently used libc version use the following command:

```bash
ldd --version
```

!alert-end!

## Installation on a local Mac workstation

The first step of this process is to set up the a suitable conda environment. For this follow
the instructions [here](https://mooseframework.inl.gov/getting_started/installation/conda.html).
Next, activate the conda environment (assume `moose-torch`), clone the repository
 and navigate to the base folder of the
stochastic tools module (let's say it is in `~/projects`):

```bash
conda activate moose-torch
git clone https://github.com/grmnptr/moose.git moose-torch
cd moose-torch
git checkout torch-compile-19571
cd ~/projects/moose/modules/stochastic_tools
```

As a next step, we download a precompiled version of `libtorch` which is suitable
for our system. This is carried out by a bash script as:

```bash
./scripts/setup_libtorch.sh
```

!alert! note
The desired version of libtorch can be set by the following
argument:

```bash
./scripts/setup_libtorch.sh --version=1.8
```

!alert-end!

which creates a `libtorch` folder in the stochastic tools root directory. This folder
will contain the necessary header files and shared object files which can be used
for dynamic linking.

The last step is to compile our MOOSE with libtorch. For this, a non-unity build
is used (which is considerably slower than the default unity build) in the
following manner:

```bash
MOOSE_UNITY=false make -j 8
```

## Installation on a local Linux workstation

The difference compared to Mac machines is that the moose conda packages (compiler stack) have been
built using `libc` v2.12 meaning that it cannot be used to set up the framework with
libtorch. For this reason, we need to install PETSc and Libmesh manually, assuming that
the default compiler stack on the system uses libc 2.27+ (most modern distributions,
such as Ubuntu actually do have suitable compilers). For this first, clone the
repository, then use the corresponding scripts as:

```bash
git clone https://github.com/grmnptr/moose.git moose-torch
cd moose-torch
git checkout torch-compile-19571
./scripts/update_and_rebuild_petsc.sh
./scripts/update_and_rebuild_libmesh.sh
```

From this point on, the procedure is more or less the same as the installation
on a Mac machine. We use the same script to download a suitable
precompiled version of `libtorch`:

```bash
cd modules/stochastic_tools
./scripts/setup_libtorch.sh
```

which creates a `libtorch` folder in the stochastic tools root directory. This folder
will contain the necessary header files and shared object files which can be used
for dynamic linking.

Now, we can move on compile MOOSE with libtorch. Again, a non-unity build
is used to make sure that namespace conflicts between `libmesh` and `libtorch` are
avoided:

```bash
MOOSE_UNITY=false make -j 8
```

## Installation on a HPC machine

The installation process is very similar to the one on a local Linux machine.
However, in some cases, we may have to build our own compiler stack/mpicc.
For this, follow the instructions
[here](https://mooseframework.inl.gov/getting_started/installation/manual_installation_gcc.html).
Once this is done and we ensured that out compilers were built using glibc 2.7+,
we can proceed to clone moose, built `petsc` and `libmesh`, download `libtorch` and
built moose with `libtorch`:

```bash
git clone https://github.com/grmnptr/moose.git moose-torch
cd moose-torch
git checkout torch-compile-19571
./scripts/update_and_rebuild_petsc.sh
./scripts/update_and_rebuild_libmesh.sh
cd modules/stochastic_tools
./scripts/setup_libtorch.sh
MOOSE_UNITY=false make -j 8
```

## Testing the installation

Basic neural network trainer and surrogate classes have been implemented in the stochastic tools
module for testing purposes. For their documentation, see
[surrogates/LibtorchSimpleNNTrainer.md] and [LibtorchSimpleNNTrainer.md].
If MOOSE compiled without error messages, we can
navigate to the testing folder within the stochastic tools root
and use the following command to test the implementation:

```bash
cd test/tests/surrogates/libtorch_nn/
./../../../../run_tests -j 8
```

The expected output should look like this:

```bash
test:surrogates/libtorch_nn.train_and_evaluate ............................................................ OK
test:surrogates/libtorch_nn.train ......................................................................... OK
test:surrogates/libtorch_nn.evaluate ...................................................................... OK
test:surrogates/libtorch_nn.retrain ....................................................................... OK
...
...
```

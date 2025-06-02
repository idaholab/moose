# Getting started with `MFEM-MOOSE`

## Introduction

`MFEM-MOOSE` is a part of the `MOOSE` framework which utilizes [`MFEM`](https://mfem.org) as its main Finite Element Method backend. It expands upon `MOOSE`'s base capabilities to support functionalities including but not limited to:

- High-order H(Div) and H(Curl) elements
- Various assembly levels (including matrix-free)
- GPU offloading (`CUDA` and `HIP`)
- Low-Order-Refined solvers


## Installing `MFEM-MOOSE`

To enable `MFEM-MOOSE` capabilities, it is necessary to install all of its dependencies, including `MFEM` itself and `Conduit`. You may do so by performing the following steps:

1. After cloning the `MOOSE` repository, navigate to the root directory and run the following commands:

```bash
export METHOD=opt
export MOOSE_JOBS=10

./scripts/update_and_rebuild_petsc.sh
./scripts/update_and_rebuild_libmesh.sh
./scripts/update_and_rebuild_mfem.sh
./scripts/update_and_rebuild_conduit.sh
./scripts/update_and_rebuild_wasp.sh
```

It may be necessary to include your desired configuration flags (for instance `--with-mpi`) to each script invocation. Alternatively, if you already have working `MFEM` or `Conduit` builds in a separate directory, you may set the variables `MFEM_DIR` and `CONDUIT_DIR` to their respective install paths.

2. Configure the `MOOSE` build by running

```bash
./configure --with-mfem
```

Again, you may wish to include other configuration flags.

3. Finally, build the framework, module, and tests by running

```bash
NUM_THREADS=10

cd framework
make -j $NUM_THREADS
cd ../modules
make -j $NUM_THREADS
cd ../test
make -j $NUM_THREADS
```

## Solving a problem with `MFEM-MOOSE`

Much of the syntax of the usual `MOOSE` scripts is preserved when creating scripts for `MFEM-MOOSE`. Example scripts may be found in the [kernel tests directory](/test/tests/mfem/kernels/). Here, we lay out the step-by-step process of writing a `MOOSE-MFEM` script to solve a simple diffusion problem. The full script may be found [here](/test/tests/mfem/kernels/diffusion.i). We roughly split the script into five parts: Problem, Geometry, Equation System, Integration, and Output.

### Problem

First of all, we must specify that the type of problem we wish to solve is an [`MFEMProblem`](/framework/doc/content/source/mfem/problem/MFEMProblem.md), so that we may take advantage of `MFEM` capabilities. 

```yaml
[Problem]
  type = MFEMProblem
[]
```

### Geometry - Mesh and Finite Element Spaces

Given that we wish to utilise `MFEM` as the backend, the mesh we import into the problem must be of [`MFEMMesh`](/framework/doc/content/source/mfem/mesh/MFEMMesh.md) type. Therefore, this must be specified in the parameter [!param](/Mesh/type) within the `Mesh` block.
```yaml
[Mesh]
  type = MFEMMesh
  file = ../mesh/mug.e
  dim = 3
[]
```

Then, we must set up the finite element spaces our problem will make use of. They can be of [`MFEMScalarFESpace`](/framework/doc/content/source/mfem/fespaces/MFEMScalarFESpace.md) type, which allows for continuous `H1` or discontinuous `L2` elements, or [`MFEMVectorFESpace`](/framework/doc/content/source/mfem/fespaces/MFEMVectorFESpace.md) type, thereby allowing tangentially-continuous `ND` or normally-continuous `RT` elements.
```yaml
[FESpaces]
  [H1FESpace]
    type = MFEMScalarFESpace
    fec_type = H1
    fec_order = FIRST
  []
[]
```

### Equation System - Variables, Kernels, and Boundary Conditions

Having the necessary finite element spaces, we may now set up the variables to be solved for which we will need for the problem. They should be of type [`MFEMVariable`](/framework/doc/content/source/mfem/variables/MFEMVariable.md). Each variable should also be associated with a relevant finite element space.
```yaml
[Variables]
  [concentration]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]
```

To set up the kernels corresponding to the differential equations we wish to solve, first we specify all materials we'll need within the `FunctorMaterials` block. In this case, we specify a `Substance` of type [`MFEMGenericConstantFunctorMaterial`](/framework/doc/content/source/mfem/functormaterials/MFEMGenericConstantFunctorMaterial.md). However, it is also possible to specify more general functors which may be piecewise, non-constant, and vectorial.

```yaml
[FunctorMaterials]
  [Substance]
    type = MFEMGenericConstantFunctorMaterial
    prop_names = diffusivity
    prop_values = 1.0
  []
[]
```

Then, within the `Kernels` block, we specify the weak forms to be added to our equation system. Typically, one would pick the `MFEM` integrators they wish to implement by checking the [linear form integrators page](https://mfem.org/lininteg/) and the [bilinear form integrators page](https://mfem.org/bilininteg/). Note that not all linear and bilinear forms that are available in `MFEM` have been implemented on `MOOSE-MFEM`, only the most common ones. Should you wish to implement an integrator that is not yet available, please raise an issue in the `MOOSE` repository.

If the integrator you wish to implement is available, you can specify it in the `type` parameter simply by taking its `MFEM` integrator name, swapping the word `Integrator` for `Kernel`, and appending `MFEM` to the beginning of the name. The table below shows a few examples of this naming convention:

| MFEM name      | MFEM-MOOSE name      |
| ------------- | ------------- |
| `DiffusionIntegrator` | `MFEMDiffusionKernel` |
| `CurlCurlIntegrator` | `MFEMCurlCurlKernel` |
| `VectorDomainLFIntegrator` | `MFEMVectorDomainLFKernel` |

Putting this together, our `Kernels` block might look as follows:
```yaml
[Kernels]
  [diff]
    type = MFEMDiffusionKernel
    variable = concentration
    coefficient = diffusivity
  []
[]
```

Now we set up boundary conditions. The full list of boundary conditions available may be found in the [BCs directory](/framework/doc/content/source/mfem/bcs). Here, we choose scalar Dirichlet boundary conditions, which corresponds to the [`MFEMScalarDirichletBC`](/framework/doc/content/source/mfem/bcs/MFEMScalarDirichletBC.md) type.
```yaml
[BCs]
  [bottom]
    type = MFEMScalarDirichletBC
    variable = concentration
    boundary = '1'
    value = 1.0
  []
  [low_terminal]
    type = MFEMScalarDirichletBC
    variable = concentration
    boundary = '2'
    value = 0.0
  []
[]
```

### Integration - Solver and Executioner

SOLVER
```yaml
[Preconditioner]
  [boomeramg]
    type = MFEMHypreBoomerAMG
    low_order_refined = false
  []
[]

[Solver]
  type = MFEMHypreGMRES
  preconditioner = boomeramg
  l_tol = 1e-16
  l_max_its = 1000
  print_level = 1
[]
```

EXECUTIONER
```yaml
[Executioner]
  type = MFEMSteady
  device = cpu
  assembly_level = legacy
[]
```

### Output

```yaml
[Outputs]
  active = ParaViewDataCollection
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/Diffusion
    vtk_format = ASCII
  []
[]
```


!if-end!

!else
!include mfem/mfem_warning.md
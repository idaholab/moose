# Getting started with `MFEM-MOOSE`

## Introduction

`MFEM-MOOSE` is a part of the `MOOSE` framework which utilizes [`MFEM`](https://mfem.org) as its main Finite Element Method backend. It expands upon `MOOSE`'s base capabilities to support functionalities including but not limited to:

- High-order H(Div) and H(Curl) elements
- Various assembly levels (including matrix-free)
- GPU offloading (`CUDA` and `HIP`)
- Low-Order-Refined solvers


## Installing `MFEM-MOOSE`

Installation instructions for `MFEM-MOOSE` can be found in [this page](/modules/doc/content/getting_started/installation/install_mfem.md).

## Solving a problem with `MFEM-MOOSE`

Much of the syntax of the usual `MOOSE` scripts is preserved when creating scripts for `MFEM-MOOSE`. Example scripts may be found in the [kernel tests directory](/test/tests/mfem/kernels/). Here, we lay out the step-by-step process of writing a `MFEM-MOOSE` script to solve a simple diffusion problem. The full script may be found [here](/test/tests/mfem/kernels/diffusion.i). We roughly split the script into five parts: Problem, Geometry, Equation System, Solver and Executioner, and Output.

### Problem

First of all, we must specify that the type of problem we wish to solve is an [`MFEMProblem`](MFEMProblem.md), so that we may take advantage of `MFEM` capabilities. 

!listing test/tests/mfem/kernels/diffusion.i block=/Problem

### Geometry - Mesh and Finite Element Spaces

Given that we wish to utilize `MFEM` as the backend, the mesh we import into the problem must be of [`MFEMMesh`](MFEMMesh.md) type. Therefore, this must be specified in the parameter [!param](/Mesh/type) within the `Mesh` block.

!listing test/tests/mfem/kernels/diffusion.i block=/Mesh

Then, we must set up the finite element spaces our problem will make use of. They can be of [`MFEMScalarFESpace`](MFEMScalarFESpace.md) type, which allows for continuous `H1` or discontinuous `L2` elements, or [`MFEMVectorFESpace`](MFEMVectorFESpace.md) type, thereby allowing tangentially-continuous `ND` or normally-continuous `RT` elements. 

!listing test/tests/mfem/kernels/diffusion.i block=/FESpaces

In this diffusion example, besides the usual `H1` space required to solve the system, we also include an `ND` space so that we may output the gradient of the result as an `AuxVariable` later on. However, if you only wish to visualize the scalar result, it is not necessary to include the `ND` space.

### Equation System - Variables, Kernels, and Boundary Conditions

Having created the necessary finite element spaces, we may now set up the variables to be solved for. They should be of type [`MFEMVariable`](MFEMVariable.md). Each variable should also be associated with a relevant finite element space.

!listing test/tests/mfem/kernels/diffusion.i block=/Variables

Here, if we also wish to output the gradient of the result, we can add it as an `AuxVariable` and set up its corresponding [`AuxKernel`](MFEMAuxKernel.md).

!listing test/tests/mfem/kernels/diffusion.i block=/AuxVariables AuxKernels

To set up the kernels corresponding to the differential equations we wish to solve, first we specify all materials we'll need within the `FunctorMaterials` block. In this case, we specify a `Substance` of type [`MFEMGenericConstantFunctorMaterial`](MFEMGenericConstantFunctorMaterial.md). However, it is also possible to specify more general functors which may be piecewise, non-constant, and vectorial.

!listing test/tests/mfem/kernels/diffusion.i block=/FunctorMaterials

Then, within the `Kernels` block, we specify the weak forms to be added to our equation system. Typically, one would pick the `MFEM` integrators they wish to implement by checking the [linear form integrators page](https://mfem.org/lininteg/) and the [bilinear form integrators page](https://mfem.org/bilininteg/). Note that not all linear and bilinear forms that are available in `MFEM` have been implemented in `MFEM-MOOSE`. Should you wish to implement an integrator that is not yet available, please raise an issue in the `MOOSE` repository.

If the integrator you wish to use is available, you can specify it in the `type` parameter simply by taking its `MFEM` integrator name, swapping the word `Integrator` for `Kernel`, and prepending `MFEM` to the beginning of its name. The table below shows a few examples of this naming convention:

| MFEM name      | MFEM-MOOSE name      |
| ------------- | ------------- |
| `DiffusionIntegrator` | `MFEMDiffusionKernel` |
| `CurlCurlIntegrator` | `MFEMCurlCurlKernel` |
| `VectorDomainLFIntegrator` | `MFEMVectorDomainLFKernel` |

Putting this together, our `Kernels` block might look as follows:

!listing test/tests/mfem/kernels/diffusion.i block=/Kernels

Now we set up boundary conditions. The full list of boundary conditions available may be found in the [BCs directory](source/mfem/bcs). Here, we choose scalar Dirichlet boundary conditions, which correspond to the [`MFEMScalarDirichletBC`](MFEMScalarDirichletBC.md) type.

!listing test/tests/mfem/kernels/diffusion.i block=/BCs

### Solver and Executioner

With the equation system set up, we specify how it is to be solved. Firstly, we choose a preconditioner and solver. The list of available types may be found in the [solvers directory](source/mfem/solvers). For problems with high polynomial order, setting [!param](/Solver/MFEMSolverBase/low_order_refined) to `true` may greatly increase performance, as explained [here](MFEMSolverBase.md). 

While in principle any solver may be used as main solver or preconditioner, the main limitation to keep in mind is that `Hypre` solvers may only be preconditioned by other `Hypre` solvers. Furthermore, when a `Hypre` solver has its `low_order_refined` parameter set to `true`, it ceases to be considered a `Hypre` solver for preconditioning purposes. 

!listing test/tests/mfem/kernels/diffusion.i block=/Preconditioner Solver remove=jacobi

Static and time-dependent executioners may be implemented respectively with the [`MFEMSteady`](MFEMSteady.md) and [`MFEMTransient`](MFEMTransient.md) types. If `MFEM-MOOSE` has been built with GPU offloading capabilities, it is possible to set [!param](/Executioner/MFEMExecutioner/device) to `cuda` or `hip` to make use of GPU acceleration. For GPU runs, it is advisable to choose an [!param](/Executioner/MFEMExecutioner/assembly_level) other than `legacy`, otherwise the matrix assembly step will not be offloaded. The options for [!param](/Executioner/MFEMExecutioner/assembly_level) are `legacy`, `full`, `element`, `partial`, and `none` (the latter is only available if `MFEM-MOOSE` has been built with `libCEED` support).

!listing test/tests/mfem/kernels/diffusion.i block=/Executioner

### Output

Finally, we can choose a data collection type for our result outputs. The types available may be found in the [outputs directory](source/mfem/outputs/).

!listing test/tests/mfem/kernels/diffusion.i block=/Outputs remove=VisItDataCollection ConduitDataCollection Outputs/active

!if-end!

!else
!include mfem/mfem_warning.md
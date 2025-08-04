# Getting started with MFEM-MOOSE

!if! function=hasCapability('mfem')

## Introduction

MFEM-MOOSE is a part of the MOOSE framework which utilizes [MFEM](https://mfem.org) as its main Finite Element Method backend. It expands upon MOOSE's base capabilities to support functionalities including but not limited to:

- High-order H(Div) and H(Curl) elements
- Various assembly levels (including partial assembly targeting matrix-free algorithms)
- GPU offloading (CUDA or HIP)
- Low-Order-Refined solvers


## Installing MFEM-MOOSE

Installation instructions for MFEM-MOOSE can be found in [this page](getting_started/installation/install_mfem.md optional=True).

## Solving a problem with MFEM-MOOSE

Much of the syntax of the usual MOOSE input file is preserved when creating input files for MFEM-MOOSE. For each input file block, the user can browse the syntax [page](syntax/index.md) for classes prefixed with `MFEM` or, alternatively, browse the MFEM section of the source [page](source/index.md). A selection of thermal, mechanical, electromagnetic and fluid example problems is described in a supporting [page](syntax/MFEM/examples_index.md).
Here, we lay out the step-by-step process of writing a MFEM-MOOSE input file to solve a simple steady state diffusion problem. The full input file may be found [here](/test/tests/mfem/kernels/diffusion.i). We roughly split the input file into five parts: Problem, Geometry, Equation System, Solver and Executioner, and Output.

### Problem

First of all, we must specify that the type of problem we wish to solve is an [MFEMProblem.md], so that we may take advantage of MFEM capabilities.

!listing test/tests/mfem/kernels/diffusion.i block=/Problem

### Geometry - Mesh and Finite Element Spaces

Given that we wish to utilize MFEM as the backend, the mesh we import into the problem must be of [MFEMMesh.md] type. Therefore, this must be specified in the parameter [!param](/Mesh/type) within the `Mesh` block.

!listing test/tests/mfem/kernels/diffusion.i block=/Mesh

Then, we must set up the finite element spaces our problem will make use of. They can be of [MFEMScalarFESpace.md] type, which allows for continuous `H1` or discontinuous `L2` elements, or [MFEMVectorFESpace.md] type, thereby allowing tangentially-continuous `ND` or normally-continuous `RT` elements.

!listing test/tests/mfem/kernels/diffusion.i block=/FESpaces

In this diffusion example, besides the usual `H1` space required to solve the system, we also include an `ND` space so that we may output the gradient of the result as an `AuxVariable` later on. However, if you only wish to visualize the scalar result, it is not necessary to include the `ND` space.

### Equation System - Variables, Kernels, and Boundary Conditions

Having created the necessary finite element spaces, we may now set up the variables to be solved for. They should be of type [MFEMVariable.md]. Each variable should also be associated with a relevant finite element space.

!listing test/tests/mfem/kernels/diffusion.i block=/Variables

Here, if we also wish to output the gradient of the result, we can add it as an `AuxVariable` and set up its corresponding [`AuxKernel`](MFEMAuxKernel.md).

!listing test/tests/mfem/kernels/diffusion.i block=/AuxVariables AuxKernels

Then, within the `Kernels` block, we specify the weak forms to be added to our equation system. Typically, one would pick the MFEM integrators they wish to implement by checking the [linear form integrators page](https://mfem.org/lininteg/) and the [bilinear form integrators page](https://mfem.org/bilininteg/). Note that not all linear and bilinear forms that are available in MFEM have been implemented in MFEM-MOOSE. Should you wish to implement an integrator that is not yet available, please submit a [feature request](https://github.com/idaholab/moose/issues/new?template=feature-request.yml).

If the integrator you wish to use is available, you can specify it in the `type` parameter simply by taking its MFEM integrator name, swapping the word `Integrator` for `Kernel`, and prepending MFEM to the beginning of its name. The table below shows a few examples of this naming convention:

| MFEM name      | MFEM-MOOSE name      |
| ------------- | ------------- |
| `DiffusionIntegrator` | `MFEMDiffusionKernel` |
| `CurlCurlIntegrator` | `MFEMCurlCurlKernel` |
| `VectorDomainLFIntegrator` | `MFEMVectorDomainLFKernel` |

Putting this together, our `Kernels` block might look as follows:

!listing test/tests/mfem/kernels/diffusion.i block=/Kernels

Now we set up boundary conditions. Here, we choose scalar Dirichlet boundary conditions, which correspond to the [MFEMScalarDirichletBC.md] type.

!listing test/tests/mfem/kernels/diffusion.i block=/BCs

### Solver and Executioner

With the equation system set up, we specify how it is to be solved. Firstly, we choose a preconditioner and solver. For problems with high polynomial order, setting [!param](/Solver/MFEMHypreGMRES/low_order_refined) to `true` may greatly increase performance, as explained [here](MFEMSolverBase.md).

While in principle any solver may be used as the main solver or preconditioner, the main limitation to keep in mind is that Hypre solvers may only be preconditioned by other Hypre solvers. Furthermore, when a Hypre solver has its `low_order_refined` parameter set to `true`, it ceases to be considered a Hypre solver for preconditioning purposes.

!listing test/tests/mfem/kernels/diffusion.i block=/Preconditioner Solver remove=jacobi

Static and time-dependent executioners may be implemented respectively with the [MFEMSteady.md] and [MFEMTransient.md] types. If MFEM-MOOSE has been built with GPU offloading capabilities, it is possible to set [!param](/Executioner/MFEMSteady/device) to `cuda` or `hip` to make use of GPU acceleration. For GPU runs, it is advisable to choose an [!param](/Executioner/MFEMSteady/assembly_level) other than `legacy`, otherwise the matrix assembly step will not be offloaded. The options for [!param](/Executioner/MFEMSteady/assembly_level) are `legacy`, `full`, `element`, `partial`, and `none` (the latter is only available if MFEM-MOOSE has been built with libCEED support).

!listing test/tests/mfem/kernels/diffusion.i block=/Executioner

### Output

Finally, we can choose a data collection type for our result outputs.

!listing test/tests/mfem/kernels/diffusion.i block=/Outputs remove=VisItDataCollection ConduitDataCollection Outputs/active

!if-end!

!else
!include mfem/mfem_warning.md

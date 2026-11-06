# Part 1: Why Kokkos-MOOSE id=kokkos_workshop_intro

!---

## The Hardware Problem

- Most of the floating-point throughput of new HPC systems comes from GPUs, and those GPUs come
  from several vendors (NVIDIA, AMD, Intel)
- Each vendor has its own native programming model: CUDA, HIP, SYCL
- Writing and maintaining a MOOSE application once per vendor is not an option

[Kokkos](https://kokkos.org) is a C++ performance-portability library. You write a parallel loop
body once and Kokkos compiles it for the selected backend:

```text
               +--> CUDA    (NVIDIA)
               +--> HIP     (AMD)
Kokkos code ---+--> SYCL    (Intel)
               +--> OpenMP  (CPU threads)
               +--> Serial
```

!---

## What Kokkos-MOOSE Is

Kokkos-MOOSE is a GPU-portable implementation of selected MOOSE object systems inside the
framework, designed to look like the MOOSE you already know.

- +Same input syntax+: Kokkos objects live in the usual blocks (`[Kernels]`, `[BCs]`,
  `[Materials]`, ...) and are selected with `type = KokkosDiffusion` etc.
- +Same infrastructure+: libMesh mesh and `DofMap`, MOOSE executioners, PETSc solvers, outputs,
  MultiApps, Controls
- +Similar C++ API+: `validParams()`, `InputParameters`, `computeQpResidual()`,
  `declareProperty`-style material properties, coupling methods
- +What changes+: how hook methods are declared, how data is stored and accessed, and a few rules
  imposed by the GPU

!alert note
Kokkos-MOOSE is not a separate finite element backend like MFEM-MOOSE. It reuses libMesh finite
element data and moves the assembly loops to the device.

!---

## What Runs Where

```text
            HOST (CPU)                                DEVICE (GPU)
  ---------------------------------         ------------------------------------
  Parse input, construct objects
  Build libMesh mesh, DofMap        ---->   Kokkos mesh, quadrature, shape data
  Executioner time loop
  PETSc nonlinear/linear solve      <---->  Residual and Jacobian assembly
                                            (Kernels, BCs, NodalKernels)
                                            Material property evaluation
                                            AuxKernels, UserObjects, Postprocessors
  Output, MultiApps, host objects
```

- Objects are +constructed on the host+, then +copied to the device+ each time a parallel loop is
  launched
- The solution vector is synchronized to the device before assembly; assembled residuals and
  Jacobians are inserted into PETSc vectors and matrices

!---

## How a Kokkos Kernel Is Executed

The base class launches one parallel loop over elements (optionally several threads per element):

```cpp
// Moose::Kokkos::Kernel::computeResidual() on the host (simplified)
_thread.resize(_num_local_residual_threads, numKokkosBlockElements());
Policy policy(0, _thread.size());
_residual_dispatcher->parallelFor(policy); // copies the kernel to the device, runs parallel_for
```

Each device thread builds a small `AssemblyDatum` for its element and runs the quadrature loop:

```cpp
// Moose::Kokkos::Kernel::operator() on the device (simplified)
auto elem = kokkosBlockElementID(_thread(tid, 1));
AssemblyDatum datum(elem, ...);
// loops over qp and i and calls YOUR computeQpResidual<Derived>(i, qp, datum)
kernel.computeResidualInternal(kernel, datum);
```

You only write `computeQpResidual()` and friends, exactly as in MOOSE.

!---

## Supported Systems

!row!
!col! width=50%

- Kernels: regular, `KernelValue`, `KernelGrad`, time, vector FE, AD
- NodalKernels
- BCs: `IntegratedBC`, `IntegratedBCValue`, `NodalBC`, AD versions
- Materials: stateful, on-demand, `constant_on`, multi-dimensional properties
- AuxKernels: elemental and nodal
- Functions: constant, parsed, piecewise

!col-end!

!col! width=50%

- UserObjects: element, side, nodal, general
- Postprocessors, VectorPostprocessors, Reporters (reducers)
- Linear finite volume: `KokkosLinearFVDiffusion`, `KokkosLinearFVAdvection`,
  `KokkosLinearFVSource`, Dirichlet/Neumann BCs
- Framework features: vector tags, eigenvalue solves, scalar variable coupling, restart,
  multiple solver systems

!col-end!
!row-end!

Framework objects are named after their MOOSE counterparts with a `Kokkos` prefix:
`KokkosDiffusion`, `KokkosDirichletBC`, `KokkosGenericConstantMaterial`, ... The heat transfer
module provides `KokkosHeatConduction`, `KokkosHeatConductionTimeDerivative`,
`KokkosHeatConductionMaterial`, and `KokkosConvectiveHeatFluxBC`.

!---

## Separate Compilation: the `.K` Extension

!row!
!col! width=55%

- Kokkos code goes in source files ending in `.K`
- `.K` files are compiled by the device compiler (`nvcc`, `hipcc`, `icpx`) into separate shared
  libraries; everything else is compiled by the usual host compiler
- If MOOSE is configured without Kokkos, `.K` files are ignored, so an application stays
  portable to systems without Kokkos
- The build finds `*.K` files under `src/` of the framework, modules, and your application
  automatically

!col-end!

!col! width=45%

!media syntax/Kokkos/kokkos_separate_compile.png style=width:100% alt=Separate compilation of Kokkos source files into device libraries

!col-end!
!row-end!

!---

## Preprocessor Guards

| Macro | Defined when | Use it for |
| :- | :- | :- |
| `MOOSE_KOKKOS_ENABLED` | Kokkos is enabled, in all files | Kokkos-related code in regular `.C` files and headers |
| `MOOSE_KOKKOS_SCOPE` | Compiling a `.K` file | `KOKKOS_FUNCTION` definitions in headers shared with `.C` files |
| `MOOSE_ENABLE_KOKKOS_GPU` | A GPU backend is enabled | Code that only makes sense on a GPU |

!alert warning title=Never change the class layout under MOOSE_KOKKOS_SCOPE
Do not guard +member variables+ or +virtual functions+ with `MOOSE_KOKKOS_SCOPE`. The host and
device compilers would see classes of different sizes, which corrupts memory silently.

!---

## Running on a GPU

```bash
$ mpiexec -n 8 ./kokkos_training-opt -i input.i
```

- Every MPI rank uses one GPU; ranks on a node are assigned to the node's GPUs round-robin
  (local rank modulo number of GPUs)
- Several ranks per GPU are allowed and often help, because host work (setup, PETSc, output)
  still runs per rank
- Use a GPU-aware MPI library for best performance
- Optionally keep PETSc vectors and matrices in device memory too:

```text
[Executioner]
  petsc_options_iname = '-vec_type -nl0_mat_type'
  petsc_options_value = 'kokkos    hypre'
[]
```

The matrix option carries the nonlinear system prefix (`nl0` by default).

!---

# Exercise 1: Run and Convert an Input File id=kokkos_workshop_ex1

!---

## Exercise 1: The Starting Input

Copy `tutorials/kokkos_workshop/kokkos_training/tests/ex1/ex1.i`. It uses only host MOOSE objects.

!style! fontsize=80%

!listing tutorials/kokkos_workshop/kokkos_training/tests/ex1/ex1.i link=False

!style-end!

!---

## Exercise 1: Tasks

1. Run `ex1.i` and keep `ex1_out.csv`
1. Copy it to `ex1_kokkos.i` and replace +every+ object with its Kokkos counterpart

   - List what is available: `./kokkos_training-opt --registry | grep Kokkos`

1. Run `ex1_kokkos.i` and compare the postprocessor values with the host run
1. Compare the `perf_graph` tables: where did the assembly time go?
1. +GPU only+: add the PETSc device vector/matrix options from the previous part and run again
1. +GPU only+: run with 1, 2, and 4 MPI ranks per GPU and compare wall times

!alert tip
On a small mesh the GPU run can be slower: launch overhead and host-device transfers dominate.
Increase `nx` and `ny` until the device starts to pay off.

!--

## Exercise 1: Solution

`tutorials/kokkos_workshop/kokkos_training/tests/ex1/ex1_kokkos.i`; the blocks that change:

!style! fontsize=80%

!listing tutorials/kokkos_workshop/kokkos_training/tests/ex1/ex1_kokkos.i block=Kernels BCs Postprocessors link=False

!style-end!

- The postprocessor values agree with the host run; the `ex1/kokkos` test checks this against
  the host gold file
- Host and Kokkos objects can coexist in one input, with exceptions (e.g. dependencies between
  host and Kokkos user objects are not resolved automatically); converting all objects in a
  system avoids host-device traffic

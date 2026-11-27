# Part 7: Performance, Debugging, and Limitations id=kokkos_workshop_perf

!---

## Performance Checklist

- +Keep the hot path on the device+: convert every residual object, material, and AuxKernel of a
  system. Host objects in the loop force data back and forth.
- +Use the optimized bases+: `KernelValue`, `KernelGrad`, `TimeKernelValue`, `IntegratedBCValue`
- +Avoid virtual dispatch+: retrieve functions and user objects by concrete type
- +No device allocation+; avoid `Moose::Kokkos::Map` in hot loops
- +Shrink material storage+: `constant_on`, on-demand properties, nothing unused on faces
- +Pick the right reduction+: reducers for small dense buffers, atomics for sparse ones
- +Tune+ `num_local_residual_threads` / `num_local_jacobian_threads` for high-order elements
- +Size the problem+: a GPU needs a large number of elements per device to be kept busy; small
  problems are dominated by launch and transfer overhead
- +Keep the solver on the device too+: PETSc `-vec_type kokkos` and a device matrix type, with a
  GPU-capable preconditioner (e.g. hypre BoomerAMG)

!---

## Debugging Workflow

1. +Debug on the CPU backend first+: `./configure --with-kokkos=cpu` runs the same Kokkos code
   through OpenMP, so `gdb`/`lldb`, sanitizers, and `METHOD=dbg` work normally
1. +Compare against the host object in the same run+: put the host and Kokkos versions in separate
   solver systems and diff their postprocessors
1. +Test the Jacobian+: `-snes_test_jacobian`, or a `PetscJacobianTester` test. Catches misspelled
   hooks and wrong derivatives.
1. +On the GPU+:

   - `::Kokkos::printf()` from device code; `::Kokkos::abort("message")` to stop with a message
   - `compute-sanitizer` (CUDA) for out-of-bounds and race detection
   - [Kokkos Tools](https://github.com/kokkos/kokkos-tools) (`KOKKOS_TOOLS_LIBS=...`) for kernel
     timers and memory profiles

!--

## Host vs. Kokkos in One Input

From `tests/ex1/ex1_compare.i` of the solutions application; the `difference` postprocessor
reports the gap between the two solutions (roundoff, here):

!listing tutorials/kokkos_workshop/kokkos_training/tests/ex1/ex1_compare.i block=Problem /Variables Kernels/diff_host Kernels/diff_kokkos Postprocessors link=False

!---

## Testing Kokkos Objects

```text
[Tests]
  [diffusion]
    type = Exodiff
    input = 'my_kokkos_test.i'
    exodiff = 'my_kokkos_test_out.e'
    capabilities = 'kokkos'
    compute_devices = 'cpu cuda xpu'
    requirement = 'The system shall ...'
  []
[]
```

- `capabilities = 'kokkos'` skips the test on builds without Kokkos
- Tests run only on the devices listed in `compute_devices` (default: CPU); GPU test runs use
  `./run_tests --compute-device=cuda`
- Reuse one test for parallel coverage with the usual TestHarness options (`-p`, `--n-threads`,
  `--recover`)

!---

## Current Limitations

- No mesh adaptivity
- No AD material properties or AD functions; AD is limited to kernels and boundary conditions
- No functor, interface, or discrete materials; no material property output
- No boundary-restricted elemental AuxKernels; no vector or array AuxKernels
- Type-erased functions and virtual user object APIs do not work on GPUs (no relocatable device
  code yet); use concrete types
- Dependencies between Kokkos and host user objects need `execution_order_group`
- Device mesh data is limited to what Kokkos-MOOSE exposes through `kokkosMesh()` and `datum`

Coverage grows every release; check the MOOSE newsletter for updates.

!---

## Where to Go Next

- Getting started and programming practices:
  [mooseframework.inl.gov/syntax/Kokkos](https://mooseframework.inl.gov/syntax/Kokkos/index.html)
- One page per system: `syntax/KokkosKernels`, `KokkosMaterials`, `KokkosBCs`,
  `KokkosAuxKernels`, `KokkosFunctions`, `KokkosUserObjects`, `KokkosNodalKernels`
- Installation: [Kokkos installation](https://mooseframework.inl.gov/getting_started/installation/install_kokkos.html)
- Reference implementations:

  - `framework/include/kokkos/` and `framework/src/kokkos/`
  - `test/include/kokkos/` (more unusual cases: stateful, multi-dimensional, on-demand)
  - `modules/heat_transfer/include/kokkos/`

- Questions: [GitHub Discussions](https://github.com/idaholab/moose/discussions)

!---

## Summary

- Kokkos-MOOSE keeps the MOOSE input syntax and object model, and moves assembly to the device
- Porting is mostly mechanical: new base class and registration macro, templated `const` hooks
  with `(datum, qp)` arguments, by-value data members
- The bugs that matter compile cleanly: references, host containers, shared scratch members,
  misspelled hooks. Test Jacobians and compare against host objects.
- Prefer the optimized bases, keep whole systems on the device, and watch material memory

!style halign=center fontsize=150%
Questions?

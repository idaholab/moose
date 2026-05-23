# SIMPLE FV Gradient Scheme Design

This page outlines a proposed design for adding flux-reconstructed pressure gradients to
the linear finite volume SIMPLE implementation while making gradient handling more modular.
The immediate target is the oscillation-free flux reconstruction approach of
[!cite](aguerre2018oscillation), but the design should also accommodate standard
Green-Gauss gradients and future limited-gradient variants.

## Assumptions

- The first implementation targets the general linear finite volume SIMPLE executioner,
  not only porous media applications.
- Existing input files should keep the current Green-Gauss pressure-gradient behavior
  unless a different gradient scheme is selected explicitly.
- Kernels and boundary conditions should read gradients that have already been assembled;
  they should not trigger gradient recomputation during residual/Jacobian assembly.
- The implementation should build on the existing linear finite volume gradient storage
  instead of introducing a separate pressure-gradient cache owned by the Rhie-Chow user
  object.

## Goals

- Provide a named gradient-scheme mechanism for cell-centered finite volume gradients.
- Allow the pressure gradient used by SIMPLE momentum kernels to be selected from the
  input file.
- Support at least the current Green-Gauss gradient and a SIMPLE flux-reconstructed
  pressure gradient.
- Make the update timing explicit in the SIMPLE iteration loop.
- Preserve current default behavior and convergence rates.

## Non-goals

- Replacing every gradient path in MOOSE in the first pull request.
- Adding new limiters beyond what is needed to expose the reconstructed pressure-gradient
  algorithm.
- Refactoring unrelated SIMPLE, Rhie-Chow, or porous-media code.

## Proposed User Input

The exact block name may change during implementation, but the desired user-facing model is
that gradients are declared as named schemes and consumers request a scheme by name:

!listing language=hit
[FVGradientSchemes]
  [p_green_gauss]
    type = GreenGauss
    variable = pressure
  []

  [p_reconstructed]
    type = SIMPLEFluxReconstructedPressure
    variable = pressure
    base_gradient = p_green_gauss
    rhie_chow_user_object = rc
    relaxation = 0.02
    update_phase = POST_FACE_FLUX_UPDATE
  []
[]

[LinearFVKernels]
  [pressure]
    type = LinearFVMomentumPressure
    variable = vel_x
    pressure = pressure
    pressure_gradient = p_reconstructed
  []
[]

If no gradient is specified, the pressure kernel should use the current Green-Gauss path.

## Architecture

### Gradient Scheme Registry

Extend the existing linear finite volume gradient infrastructure into a registry of named
cell-gradient fields. Each entry should identify:

- the variable whose gradient is represented,
- the gradient algorithm,
- the storage for cell-centered gradient values,
- the phase in which the gradient must be updated,
- any producer-specific dependencies such as a Rhie-Chow mass flux user object.

The registry should own or reference the storage so consumers can request gradients by name
without knowing which object produced them.

### Gradient Scheme Interface

Introduce a small interface for gradient producers. A scheme should be responsible for:

- declaring its variable dependency,
- declaring its update phase,
- assembling its cell-gradient field,
- exposing read-only access to the assembled field.

The interface should avoid lazy assembly from kernel calls. This keeps update order visible
and makes future thread-safety and MPI behavior easier to reason about.

### SIMPLE Update Phases

SIMPLE should explicitly update registered schemes at well-defined phases. Candidate phases
are:

- +PRE_MOMENTUM_ASSEMBLY+ for gradients required by the momentum equation,
- +POST_MOMENTUM_SOLVE+ for gradients depending on updated momentum variables,
- +POST_FACE_FLUX_UPDATE+ for gradients depending on Rhie-Chow face fluxes,
- +PRE_PRESSURE_ASSEMBLY+ for gradients required by the pressure equation,
- +POST_PRESSURE_SOLVE+ for gradients depending on the updated pressure field.

The reconstructed pressure-gradient scheme should initially update after the Rhie-Chow face
flux and cell-velocity information is current.

### Green-Gauss Scheme

Wrap the current default cell-gradient computation as a named Green-Gauss scheme. This is
the compatibility path and should be the default for pressure gradients when users do not
select another scheme.

### Reconstructed Pressure Scheme

Implement the Aguerre-style pressure-gradient reconstruction as a pressure-gradient scheme.
At a high level, the scheme should:

1. reconstruct the cell velocity from face fluxes,
2. use the momentum relation to infer the pressure-gradient correction,
3. optionally relax the reconstructed gradient toward the base gradient,
4. publish the final cell-centered pressure gradient through the registry.

The scheme may depend on Rhie-Chow data, but that dependency should be isolated in the
scheme implementation. Momentum kernels should only see a named gradient field.

### Kernel Consumption

`LinearFVMomentumPressure` should request a pressure-gradient field from the registry. It
should not depend directly on the Rhie-Chow mass flux object. This keeps the kernel stable as
new gradient algorithms are added.

## Implementation Plan

1. Audit existing gradient flow.
   Verify where raw and limited finite volume gradients are stored, updated, and consumed.
2. Define the named gradient-scheme interface.
   Verify that Green-Gauss and reconstructed pressure gradients can both satisfy it.
3. Extend gradient storage into a registry.
   Verify that multiple gradients for the same variable can coexist without changing default
   behavior.
4. Add input-level scheme declaration and lookup.
   Verify that kernels can request a gradient by name and receive clear errors for invalid
   names.
5. Add explicit SIMPLE update scheduling.
   Verify that each registered scheme updates exactly at its declared phase.
6. Implement the Green-Gauss scheme wrapper.
   Verify that existing SIMPLE tests are unchanged with default inputs.
7. Implement the reconstructed pressure-gradient scheme.
   Verify that the reconstruction uses current Rhie-Chow flux and momentum data without
   adding kernel-level Rhie-Chow coupling.
8. Wire `LinearFVMomentumPressure` to the registry.
   Verify that the selected pressure gradient is used in momentum assembly.
9. Document public headers and user-facing syntax.
   Verify that the responsibilities of schemes, storage, and consumers are clear.
10. Add MMS tests for default and reconstructed schemes.
    Verify that reconstructed gradients do not degrade the existing observed convergence
    rates on orthogonal and nonorthogonal meshes.

## Validation Plan

- Run existing linear finite volume SIMPLE tests with default inputs to check backward
  compatibility.
- Add MMS comparisons for Green-Gauss and reconstructed pressure gradients.
- Include orthogonal and nonorthogonal meshes in the first test set.
- Add follow-up coverage for dimensionality, variable density, boundary conditions, MPI
  partitioning, and porous-media interaction after the base design is stable.

## Open Questions

- Whether gradient schemes should be implemented as user objects, system-owned helpers, or
  a new MooseObject family.
- Whether `update_phase` should be user-selectable for all schemes or fixed by each scheme
  type.
- How much of the existing limited-gradient map should be migrated immediately versus
  wrapped for compatibility.
- What default relaxation should be used for the reconstructed pressure gradient across a
  broad set of flow regimes.

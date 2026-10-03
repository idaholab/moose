# LinearFVPressureCorrectionDiffusionJump

!syntax description /LinearFVKernels/LinearFVPressureCorrectionDiffusionJump

## Description

`LinearFVPressureCorrectionDiffusionJump` extends
[LinearFVPressureCorrectionDiffusion.md] with the pressure
jump supplied by a [PorousRhieChowMassFlux.md] object. On a porous baffle
face, a transmissibility $T_f$ and signed jump $J_f$ add $T_f J_f$ to the right-hand side. Ordinary
internal and boundary faces retain the base pressure-correction discretization.

Set
[!param](/LinearFVKernels/LinearFVPressureCorrectionDiffusionJump/rhie_chow_user_object) to the
same user object used by the [SIMPLE.md] executioner. When
[!param](/LinearFVKernels/LinearFVPressureCorrectionDiffusionJump/use_two_term_pressure_expansion)
is enabled, the baffle transmissibility is evaluated from the lagged two-term reconstructed
pressure expansion; this option requires
[FVReconstructedPressureGradient.md].

## Example Input Syntax

In this example, `LinearFVPressureCorrectionDiffusionJump` supplies the pressure-correction
diffusion operator across two porous interfaces.

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/linear-segregated/1d-simple-channel/porous-baffle-1d.i block=LinearFVKernels/p_diffusion

!syntax parameters /LinearFVKernels/LinearFVPressureCorrectionDiffusionJump

!syntax inputs /LinearFVKernels/LinearFVPressureCorrectionDiffusionJump

!syntax children /LinearFVKernels/LinearFVPressureCorrectionDiffusionJump

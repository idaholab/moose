# LinearFVAnisotropicDiffusionJump

!syntax description /LinearFVKernels/LinearFVAnisotropicDiffusionJump

This pressure-correction diffusion kernel extends
[LinearFVPressureCorrectionDiffusion.md] with the pressure jump supplied by a
[PorousRhieChowMassFlux.md] object. On a porous baffle face, a transmissibility $T_f$ and signed
jump $J_f$ add $T_f J_f$ to the right-hand side. Ordinary internal and boundary faces retain the
base diffusion discretization.

Set [!param](/LinearFVKernels/LinearFVAnisotropicDiffusionJump/rhie_chow_user_object) to the same
user object used by the [SIMPLE.md] executioner. When
[!param](/LinearFVKernels/LinearFVAnisotropicDiffusionJump/use_two_term_pressure_expansion) is
enabled, the baffle transmissibility is evaluated from the lagged two-term reconstructed pressure
expansion; this option requires [FVReconstructedPressureGradient.md].

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/linear-segregated/1d-simple-channel/porous-baffle-1d.i block=LinearFVKernels/p_diffusion

!syntax parameters /LinearFVKernels/LinearFVAnisotropicDiffusionJump

!syntax inputs /LinearFVKernels/LinearFVAnisotropicDiffusionJump

!syntax children /LinearFVKernels/LinearFVAnisotropicDiffusionJump

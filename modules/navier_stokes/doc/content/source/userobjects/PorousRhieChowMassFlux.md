# PorousRhieChowMassFlux

!syntax description /UserObjects/PorousRhieChowMassFlux

## Overview

`PorousRhieChowMassFlux` extends [RhieChowMassFlux.md] for porous-medium and
porous-baffle calculations in the linear finite volume SIMPLE workflow.

Compared with the base object, it adds:

- cell-porosity scaling of the pressure-coupling fields,
- harmonic interpolation of the pressure-diffusion coefficient by default,
- porous-baffle pressure jumps stored and relaxed on coupled interface faces,
- compatibility with [FVPressureJumpGreenGaussGradient](FVPressureJumpGreenGaussGradient.md), which
  reconstructs the pressure separately on each side of every baffle, and
- a pressure-velocity coupling gradient that remains separate from the solution gradient used by
  the baffle diffusion correction.

Pressure jumps are supplied by the models listed in
[!param](/UserObjects/PorousRhieChowMassFlux/pressure_jump_models), such as
[BernoulliFormLossPressureJump.md]. The jump is updated from the
current face mass flux and under-relaxed with
[!param](/UserObjects/PorousRhieChowMassFlux/pressure_jump_relaxation). See
[porous_rhie_chow_baffle.md] for the discrete coupling.

Before each pressure corrector, the object updates the jump from the preceding corrected face flux.
The pressure variable's [FVPressureJumpGreenGaussGradient](FVPressureJumpGreenGaussGradient.md)
then removes that jump when interpolating pressure across each baffle. A zero jump recovers the
standard Green-Gauss face value exactly.

This is the user object expected by
[LinearPWCNSFVMomentumFlux.md],
[LinearFVPressureCorrectionDiffusionJump.md], and porous
uses of
[LinearFVEnergyAdvection.md].

In this example, `PorousRhieChowMassFlux` uses a separately configured pressure-jump model on two
porous interfaces.

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/linear-segregated/1d-simple-channel/porous-baffle-1d.i block=UserObjects

!syntax parameters /UserObjects/PorousRhieChowMassFlux

!syntax inputs /UserObjects/PorousRhieChowMassFlux

!syntax children /UserObjects/PorousRhieChowMassFlux

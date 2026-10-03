# PorousRhieChowMassFlux

!syntax description /UserObjects/PorousRhieChowMassFlux

## Overview

`PorousRhieChowMassFlux` extends [RhieChowMassFlux.md] for porous-medium and
porous-baffle calculations in the linear finite volume SIMPLE workflow.

Compared with the base object, it adds:

- cell-porosity scaling of the pressure-coupling fields,
- porous-baffle pressure jumps stored and relaxed on coupled interface faces,
- corrected and optionally reconstructed pressure gradients consistent with the
  porous/baffle pressure operator.

Pressure jumps are supplied by the models listed in
[!param](/UserObjects/PorousRhieChowMassFlux/pressure_jump_models), such as
[BernoulliFormLossPressureJump.md]. The jump is updated from the
current face mass flux and under-relaxed with
[!param](/UserObjects/PorousRhieChowMassFlux/pressure_jump_relaxation). See
[porous_rhie_chow_baffle.md] for the discrete coupling.

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

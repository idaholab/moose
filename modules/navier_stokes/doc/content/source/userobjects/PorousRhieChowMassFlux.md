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

The pressure jump is updated from the current face mass flux and under-relaxed with
[!param](/UserObjects/PorousRhieChowMassFlux/pressure_baffle_relaxation). Optional form losses are
specified with [!param](/UserObjects/PorousRhieChowMassFlux/baffle_form_loss). See
[porous_rhie_chow_baffle.md] for the discrete coupling.

This is the user object expected by [PorousLinearWCNSFVMomentumFlux.md],
[LinearFVAnisotropicDiffusionJump.md], and porous uses of [LinearFVEnergyAdvection.md].

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/linear-segregated/1d-simple-channel/porous-baffle-1d.i block=UserObjects/rc

!syntax parameters /UserObjects/PorousRhieChowMassFlux

!syntax inputs /UserObjects/PorousRhieChowMassFlux

!syntax children /UserObjects/PorousRhieChowMassFlux

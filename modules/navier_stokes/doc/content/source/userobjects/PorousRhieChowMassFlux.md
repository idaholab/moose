# PorousRhieChowMassFlux

!syntax description /UserObjects/PorousRhieChowMassFlux

## Overview

`PorousRhieChowMassFlux` extends [RhieChowMassFlux.md] for porous-medium and
porous-baffle calculations in the linear finite volume SIMPLE workflow.

Compared with the base object, it adds:

- porous-aware face interpolation for advected quantities,
- cell-porosity scaling of the pressure-coupling fields,
- porous-baffle pressure jumps stored and relaxed on coupled interface faces,
- corrected and optionally reconstructed pressure gradients consistent with the
  porous/baffle pressure operator.

This is the user object expected by porous linear-FV kernels such as
`PorousLinearWCNSFVMomentumFlux` and [LinearFVEnergyAdvection.md].

!syntax parameters /UserObjects/PorousRhieChowMassFlux

!syntax inputs /UserObjects/PorousRhieChowMassFlux

!syntax children /UserObjects/PorousRhieChowMassFlux

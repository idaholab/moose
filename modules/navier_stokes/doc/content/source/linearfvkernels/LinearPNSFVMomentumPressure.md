!description /LinearFVKernels/LinearPNSFVMomentumPressure

## Description

`LinearPNSFVMomentumPressure` multiplies the pressure gradient source that is
added to the segregated momentum predictor by the local porosity.  This kernel
should be used together with `PorousRhieChowMassFlux` and the porous mass
balance kernels when the unknown velocity is the interstitial velocity,
yielding the porous momentum balance

$$
\nabla \cdot \left( \epsilon \mu \nabla \mathbf{u} \right) =
 - \epsilon \nabla p + \cdots
$$

Only the right-hand side contribution is affected, so the kernel can be
combined with the existing linear FV momentum flux kernels by supplying a
porosity-weighted viscosity functor.

## Example Input File

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/porous_linear_basic.i
  block=LinearFVKernels/momentum_pressure
  caption=Porosity-weighted pressure gradient term.

## Parameters

!parameters /LinearFVKernels/LinearPNSFVMomentumPressure


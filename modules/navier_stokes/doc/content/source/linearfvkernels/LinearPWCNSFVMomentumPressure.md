# LinearPWCNSFVMomentumPressure

!syntax description /LinearFVKernels/LinearPWCNSFVMomentumPressure

## Description

`LinearPWCNSFVMomentumPressure` adds the pressure-gradient contribution
$-\epsilon\nabla p$ to a porous momentum equation written in superficial velocity. The
[!param](/LinearFVKernels/LinearPWCNSFVMomentumPressure/porosity) functor supplies $\epsilon$.
The pressure gradient is evaluated in the same way as in
[LinearFVMomentumPressure.md], including support for reconstructed
pressure gradients.

## Example Input Syntax

In this example, `LinearPWCNSFVMomentumPressure` adds the x-directed pressure force to the
superficial-velocity equation.

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/linear-segregated/1d-simple-channel/porous-baffle-1d.i block=LinearFVKernels/u_pressure

!syntax parameters /LinearFVKernels/LinearPWCNSFVMomentumPressure

!syntax inputs /LinearFVKernels/LinearPWCNSFVMomentumPressure

!syntax children /LinearFVKernels/LinearPWCNSFVMomentumPressure

# LinearPWCNSFVMomentumFlux

!syntax description /LinearFVKernels/LinearPWCNSFVMomentumFlux

## Description

`LinearPWCNSFVMomentumFlux` specializes
[LinearWCNSFVMomentumFlux.md] for momentum equations whose unknown is
superficial velocity. The [PorousRhieChowMassFlux.md] user object provides
the face mass flux and the porosity on each side of the face. The advection contribution assembled
into a cell row is scaled by that cell's inverse porosity, so the mass flux transports interstitial
velocity.

At a porous baffle, the Rhie-Chow object can require one-sided reconstruction. In that case this
kernel uses the local velocity state for each side of the advection operator. The viscous stress
operator remains continuous across the interface. Set
[!param](/LinearFVKernels/LinearPWCNSFVMomentumFlux/use_two_point_stress_transmissibility) to use
the jump-safe harmonic two-point stress treatment instead of the standard face-interpolated
gradient. Porosity must be positive on every active face.

## Example Input Syntax

In this example, `LinearPWCNSFVMomentumFlux` assembles the x-momentum equation for superficial
velocity in a channel containing porous interfaces.

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/linear-segregated/2d-diagonal-baffle/porous-baffle-straight-diagonal.i block=LinearFVKernels/u_advection

!syntax parameters /LinearFVKernels/LinearPWCNSFVMomentumFlux

!syntax inputs /LinearFVKernels/LinearPWCNSFVMomentumFlux

!syntax children /LinearFVKernels/LinearPWCNSFVMomentumFlux

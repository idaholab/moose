# PorousLinearWCNSFVMomentumFlux

!syntax description /LinearFVKernels/PorousLinearWCNSFVMomentumFlux

This kernel specializes [LinearWCNSFVMomentumFlux.md] for momentum equations whose unknown is
superficial velocity. The [PorousRhieChowMassFlux.md] user object provides the face mass flux and
the porosity on each side of the face. The advection contribution assembled into a cell row is
scaled by that cell's inverse porosity, so the mass flux transports interstitial velocity.

At a porous baffle, the Rhie-Chow object can require one-sided reconstruction. In that case this
kernel uses the local velocity state for each side of the advection operator. The viscous stress
operator remains continuous across the interface. Porosity must be positive on every active face.

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/linear-segregated/2d-diagonal-baffle/porous-baffle-straight-diagonal.i block=LinearFVKernels/u_advection

!syntax parameters /LinearFVKernels/PorousLinearWCNSFVMomentumFlux

!syntax inputs /LinearFVKernels/PorousLinearWCNSFVMomentumFlux

!syntax children /LinearFVKernels/PorousLinearWCNSFVMomentumFlux

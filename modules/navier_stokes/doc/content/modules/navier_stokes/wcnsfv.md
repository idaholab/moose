# Weakly Compressible Finite Volume Navier Stokes

The weakly compressible implementation of the Navier Stokes equations in finite volume
is largely based of the [incompressible version](insfv.md), except that constant densities
were replaced by functor material properties. The incompressible kernels were adapted to use
the functor material properties, so most kernels can be adapted directly. The only kernels
that different between the two formulations are the time derivative kernels, as they include
the density time derivative.

- [WCNSFVMassTimeDerivative.md]

- [WCNSFVMomentumTimeDerivative.md]

- [WCNSFVEnergyTimeDerivative.md]

In addition, the [NavierStokesFV](/Modules/NavierStokesFV/index.md)
action syntax can also be used to set up weakly-compressible simulations.

## Lid Driven Cavity Heated Flow

We show below an example input file for a cavity with a moving lid, with different heat sources
on both sides of the cavity.

!listing modules/navier_stokes/test/tests/finite_volume/ins/boussinesq/transient-wcnsfv.i

## Heated Channel Flow

We show below an example input file for a heated channel. The fluid progressively heats up through
the channel, with a volumetric heat source.

!listing modules/navier_stokes/test/tests/finite_volume/wcns/channel-flow/2d-transient.i

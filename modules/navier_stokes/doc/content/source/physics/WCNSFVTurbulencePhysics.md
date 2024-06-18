# Navier Stokes Turbulence / WCNSFVTurbulencePhysics

!syntax description /Physics/NavierStokes/Turbulence/WCNSFVTurbulencePhysics

## Mixing length turbulence model

See the [mixing length theory page](rans_theory.md) for additional information.

If the turbulence model is selected to be the mixing-length model, a field variable representing the mixing length
is computed using the [WallDistanceMixingLengthAux.md].

The turbulent dynamic viscosity is then computed using a [MixingLengthTurbulentViscosityFunctorMaterial.md].
The following kernels are then added:

- [INSFVTurbulentDiffusion.md] to the flow equation if the flow equations are being solved
- [WCNSFVMixingLengthEnergyDiffusion.md] to the fluid energy equation if the fluid energy equation is being solved
- [INSFVMixingLengthScalarDiffusion.md] to the scalar transport equations if the scalar transport is being solved

!alert note
These kernels are only added if each of these equations are being defined using their respective `Physics`.

## Coupling with other Physics

A turbulence model can be added to a heat advection solve by using both a `WCNSFVTurbulencePhysics` and a [WCNSFVFluidHeatTransferPhysics.md].
The following input performs this coupling for weakly compressible flow in a 2D channel.
No system parameters are passed, so the equations are solved in a fully coupled manner in the same [nonlinear system](systems/NonlinearSystem.md).

!listing test/tests/finite_volume/wcns/channel-flow/2d-transient-physics.i block=Physics

A turbulence model can be added to a scalar advection solve by using both a `WCNSFVTurbulencePhysics` and a [WCNSFVScalarTransportPhysics.md].
The following input performs this coupling for incompressible flow in a 2D channel.
No system parameters are passed, so the equations are solved in a fully coupled manner in the same [nonlinear system](systems/NonlinearSystem.md).

!listing test/tests/finite_volume/ins/channel-flow/2d-mixing-length-physics.i block=Physics

!syntax parameters /Physics/NavierStokes/Turbulence/WCNSFVTurbulencePhysics

!syntax inputs /Physics/NavierStokes/Turbulence/WCNSFVTurbulencePhysics

!syntax children /Physics/NavierStokes/Turbulence/WCNSFVTurbulencePhysics

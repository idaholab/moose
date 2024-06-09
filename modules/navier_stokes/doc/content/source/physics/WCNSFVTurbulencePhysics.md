# Navier Stokes Turbulence / WCNSFVTurbulencePhysics

!syntax description /Physics/NavierStokes/Turbulence/WCNSFVTurbulencePhysics

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

!tag name=WCNSFVTurbulencePhysics pairs=module:navier_stokes system:physics

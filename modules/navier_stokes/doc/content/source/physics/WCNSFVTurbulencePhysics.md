# WCNSFVTurbulencePhysics

!syntax description /Physics/WCNSFVTurbulence

See the main [WCNSFVPhysics.md] page for an overview of the syntax and capabilities of the
weakly-compressible Navier Stokes `Physics` syntax.

## Coupling with other Physics

A turbulence model can be added to a heat advection solve by using both a `WCNSFVTurbulencePhysics` and a [WCNSFVHeatAdvectionPhysics.md].
The following input performs this coupling for incompressible flow in a 2D channel.
No system parameters are passed, so the equations are solved in a fully coupled manner in the same [nonlinear system](systems/NonlinearSystem.md).

!listing test/tests/finite_volume/ins/channel-flow/2d-transient-physics-energy.i block=Physics

A turbulence model can be added to a scalar advection solve by using both a `WCNSFVTurbulencePhysics` and a [WCNSFVScalarAdvectionPhysics.md].
The following input performs this coupling for incompressible flow in a 2D channel.
No system parameters are passed, so the equations are solved in a fully coupled manner in the same [nonlinear system](systems/NonlinearSystem.md).

!listing test/tests/finite_volume/ins/channel-flow/2d-mixing-length-physics.i block=Physics

!syntax parameters /Physics/WCNSFVTurbulence

!syntax inputs /Physics/WCNSFVTurbulence

!syntax children /Physics/WCNSFVTurbulence

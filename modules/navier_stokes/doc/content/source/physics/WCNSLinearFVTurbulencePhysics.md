# WCNSLinearFVTurbulencePhysics / Navier Stokes Turbulence

!syntax description /Physics/NavierStokes/TurbulenceSegregated/WCNSLinearFVTurbulencePhysics

## K-Epsilon turbulence model

The `WCNSLinearFVTurbulencePhysics` can be set to create the standard k-epsilon two-equation model.

The turbulent viscosity is then computed with a [kEpsilonViscosityAux.md].


The k equation is created with:

- a [LinearFVTimeDerivative.md] for the time derivative if simulating a transient
- a [LinearFVTurbulentAdvection.md] for the turbulent kinetic energy advection term
- a [LinearFVTurbulentDiffusion.md] for the turbulent kinetic energy diffusion term
- a [LinearFVTKESourceSink.md] for the turbulent kinetic energy source and dissipation (sink) terms


The epsilon equation is created with:

- a [LinearFVTimeDerivative.md] for the time derivative if simulating a transient
- a [LinearFVTurbulentAdvection.md] for the turbulent kinetic energy dissipation rate advection term
- a [LinearFVTurbulentDiffusion.md] for the turbulent kinetic energy dissipation rate diffusion term
- a [LinearFVTKEDSourceSink.md] for the turbulent kinetic energy dissipation rate source and removal (sink) terms


The boundary conditions are not set in this object for the `TKE` and `TKED` variables, as they
are computed by the wall-functions in the relevant kernels. A boundary condition is set for the turbulent
viscosity when using an auxiliary variable, with a [LinearFVTurbulentViscosityWallFunctionBC.md].


## Coupling with other Physics

A turbulence model can be added to a heat advection solve by using both a `WCNSLinearFVTurbulencePhysics` and a [WCNSLinearFVFluidHeatTransferPhysics.md].
The following input performs this coupling for incompressible flow for the standard k-epsilon turbulence model in a 2D channel.

!listing test/tests/finite_volume/ins/turbulence/channel/linear-segregated/channel_heated-physics.i block=Physics

A turbulence model can be added to a scalar advection solve by using both a `WCNSLinearFVTurbulencePhysics` and a [WCNSLinearFVScalarTransportPhysics.md].
The following input performs this coupling for incompressible flow for the mixing length turbulence model in a 2D channel.

!listing test/tests/finite_volume/ins/turbulence/channel/linear-segregated/channel_scalars-physics.i block=Physics

!syntax parameters /Physics/NavierStokes/TurbulenceSegregated/WCNSLinearFVTurbulencePhysics

!syntax inputs /Physics/NavierStokes/TurbulenceSegregated/WCNSLinearFVTurbulencePhysics

!syntax children /Physics/NavierStokes/TurbulenceSegregated/WCNSLinearFVTurbulencePhysics

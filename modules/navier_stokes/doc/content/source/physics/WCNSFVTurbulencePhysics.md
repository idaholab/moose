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

## K-Epsilon turbulence model

The `WCNSFVTurbulencePhysics` can be set to create the k-epsilon two-equation model.

The turbulent viscosity is then computed with:

- a [kEpsilonViscosityAux.md] if [!param](/Physics/NavierStokes/Turbulence/WCNSFVTurbulencePhysics/mu_t_as_aux_variable) is set to true
- a [INSFVkEpsilonViscosityFunctorMaterial.md] otherwise


The k equation is created with:

- a [FVFunctorTimeKernel.md] for the time derivative if simulating a transient
- a [INSFVTurbulentAdvection.md] for the turbulent kinetic energy advection term
- a [INSFVTurbulentDiffusion.md] for the turbulent kinetic energy diffusion term
- a [INSFVTKESourceSink.md] for the turbulent kinetic energy source and dissipation (sink) terms


The epsilon equation is created with:

- a [FVFunctorTimeKernel.md] for the time derivative if simulating a transient
- a [INSFVTurbulentAdvection.md] for the turbulent kinetic energy dissipation rate advection term
- a [INSFVTurbulentDiffusion.md] for the turbulent kinetic energy dissipation rate diffusion term
- a [INSFVTKEDSourceSink.md] for the turbulent kinetic energy dissipation rate source and removal (sink) terms


The boundary conditions are not set in this object for the `TKE` and `TKED` variables, as they
are computed by the wall-functions in the relevant kernels. A boundary condition is set for the turbulent
viscosity when using an auxiliary variable, with a [INSFVTurbulentViscosityWallFunction.md].


## Coupling with other Physics

A turbulence model can be added to a heat advection solve by using both a `WCNSFVTurbulencePhysics` and a [WCNSFVFluidHeatTransferPhysics.md].
The following input performs this coupling for weakly compressible flow for the mixing length turbulence model in a 2D channel.
No system parameters are passed, so the equations are solved in a fully coupled manner in the same [nonlinear system](systems/NonlinearSystem.md).

!listing test/tests/finite_volume/wcns/channel-flow/2d-transient-physics.i block=Physics

A turbulence model can be added to a scalar advection solve by using both a `WCNSFVTurbulencePhysics` and a [WCNSFVScalarTransportPhysics.md].
The following input performs this coupling for incompressible flow for the mixing length turbulence model in a 2D channel.
No system parameters are passed, so the equations are solved in a fully coupled manner in the same [nonlinear system](systems/NonlinearSystem.md).

!listing test/tests/finite_volume/ins/channel-flow/2d-mixing-length-physics.i block=Physics

!syntax parameters /Physics/NavierStokes/Turbulence/WCNSFVTurbulencePhysics

!syntax inputs /Physics/NavierStokes/Turbulence/WCNSFVTurbulencePhysics

!syntax children /Physics/NavierStokes/Turbulence/WCNSFVTurbulencePhysics

# WCNSFVSwitchableInletVelocityBC

!syntax description /FVBCs/WCNSFVSwitchableInletVelocityBC

The file is similar to [WCNSFVInletVelocityBC.md] but it allows the boundary conditions to be switched on/off.

There are two options for specifying a component of the inlet velocity:

- specifying a velocity postprocessor

- specifying a mass flow rate postprocessor and a density functor. The functor is
  usually a functor material property, defined by a [GeneralFunctorFluidProps.md].
  The scaling factor can be used to account for projections if the inlet flow and
  the surface are not aligned.


This boundary condition works with postprocessors, which may be replaced by constant
values in the input. The intended use case for this boundary condition is to be receiving its value from
a coupled application, using a [Receiver.md] postprocessor.

The `switch` parameter is a boolean that is used to turn the boundary condition on/off.
The parameter is controllable during runtime and can be controlled via a `BoolFunctionControl`.

The switch works as follows:

- If `switch = true`: the boundary condition is applied as described above.

- If `switch = false`: the boundary condition is not applied and a single sided extrapolation to the boundary
  is applied from internal extrapolation. The user can expect second order convergence.

In both cases, the interpolated value at the face is contoled by `face_limiter`.
By default, `face_limiter = 1.0`.
`face_limiter` value is controllable during runtime.

!alert note
Specifying the inlet velocity using a `WCNSFVSwitchableInletVelocityBC` will not preserve
mass or momentum flow at the boundary in most cases, in part because of the discretization error.
Specifying incoming mass and momentum fluxes using a [WCNSFVMassFluxBC.md] and a
[WCNSFVMomentumFluxBC.md] is currently the only conservative approach.

## Example input syntax

In this example input, the boundary conditions to the mass conservation equation and the
momentum equations are specified using two `WCNSFVSwitchableInletVelocityBC`, one for each component of the velocity.
The inlet velocity is specified using a mass flow rate postprocessor.

!listing modules/navier_stokes/test/tests/finite_volume/wcns/boundary_conditions/dirichlet_bcs_mdot.i block=FVBCs

!syntax parameters /FVBCs/WCNSFVSwitchableInletVelocityBC

!syntax inputs /FVBCs/WCNSFVSwitchableInletVelocityBC

!syntax children /FVBCs/WCNSFVSwitchableInletVelocityBC

# WCNSFVSwitchableInletVelocityBC

!syntax description /FVBCs/WCNSFVSwitchableInletVelocityBC

The file is similar to [WCNSFVInletVelocityBC.md] but it allows the boundary conditions to be switched on/off.

The `switch_bc` parameter is a boolean that is used to turn the boundary condition on/off.
The parameter is controllable during runtime and can be controlled via a `BoolFunctionControl`.

The switch works as follows:

- If `switch = true`: the boundary condition is applied as described in [WCNSFVInletVelocityBC.md].

- If `switch = false`: the boundary condition is not applied and a single sided extrapolation to the boundary
  is applied from internal extrapolation.

In both cases, the interpolated value at the face is controlled by `face_limiter`.
By default, `face_limiter = 1.0`.
`face_limiter` value is controllable during runtime.

## Example input syntax

In this example input, the boundary conditions to the mass conservation equation and the
momentum equations are specified using two `WCNSFVSwitchableInletVelocityBC`, one for each component of the velocity.
The inlet velocity is specified using a mass flow rate postprocessor.

!listing modules/navier_stokes/test/tests/finite_volume/wcns/boundary_conditions/dirichlet_bcs_mdot.i block=FVBCs

!syntax parameters /FVBCs/WCNSFVSwitchableInletVelocityBC

!syntax inputs /FVBCs/WCNSFVSwitchableInletVelocityBC

!syntax children /FVBCs/WCNSFVSwitchableInletVelocityBC

# WCNSFVInletVelocityBC

!syntax description /FVBCs/WCNSFVInletVelocityBC

There are two options for specifying a component of the inlet velocity:

- specifying a velocity postprocessor

- specifying a mass flow rate postprocessor and a density functor. The functor is
  usually a functor material property, defined by a [GeneralFunctorFluidProps.md].
  The scaling factor can be used to account for projections if the inlet flow and
  the surface are not aligned.


This boundary condition works with postprocessors, which may be replaced by constant
values in the input. The intended use case for this boundary condition is to be receiving its value from
a coupled application, using a [Receiver.md] postprocessor.

!alert note
Specifying the inlet velocity using a `WCNSFVInletVelocityBC` will not preserve
mass or momentum flow at the boundary in most cases, in part because of the discretization error.
Specifying incoming mass and momentum fluxes using a [WCNSFVMassFluxBC.md] and a
[WCNSFVMomentumFluxBC.md] is currently the only conservative approach.

## Example input syntax

In this example input, the boundary conditions to the mass conservation equation and the
momentum equations are specified using two `WCNSFVInletVelocityBC`, one for each component of the velocity.
The inlet velocity is specified using a mass flow rate postprocessor.

!listing modules/navier_stokes/test/tests/finite_volume/wcns/boundary_conditions/dirichlet_bcs_mdot.i block=FVBCs

!syntax parameters /FVBCs/WCNSFVInletVelocityBC

!syntax inputs /FVBCs/WCNSFVInletVelocityBC

!syntax children /FVBCs/WCNSFVInletVelocityBC

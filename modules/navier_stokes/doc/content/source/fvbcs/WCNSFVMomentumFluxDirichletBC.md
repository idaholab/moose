# WCNSFVMomentumFluxDirichletBC

!syntax description /FVBCs/WCNSFVMomentumFluxDirichletBC

This boundary condition is similar to [WCNSFVMassFluxBC.md], except that it enforced the mass
flow rate provided by the user as a Dirichlet boundary conditions. Hence, using this boundary
condition the mass flow rate at the inlet faces will always be the one specified by the user.

## Example input syntax

In this example input, the inlet boundary condition to the mass conservation equation is
specified using a `WCNSFVMomentumFluxDirichletBC`. The mass flux is specified using the mass flow rate
and the inlet area.

!listing modules/navier_stokes/test/tests/finite_volume/wcns/boundary_conditions/variable_deansity_channel_test.i block=FVBCs

!syntax parameters /FVBCs/WCNSFVMomentumFluxDirichletBC

!syntax inputs /FVBCs/WCNSFVMomentumFluxDirichletBC

!syntax children /FVBCs/WCNSFVMomentumFluxDirichletBC

!template load file=sqa/module_srs.md.template category=navier_stokes module=Navier Stokes

!template! item key=system-scope
!! system-scope-begin
The [!ac](MOOSE) Navier Stokes module provides numerical discretizations of the
Navier Stokes equations to model the flow of fluid through regular and porous media.
The equations discretized include the conservation of mass, momentum, energy and of transported
scalars / species.
It covers a wide range of fluids and of fluid flow regimes. It covers both natural and forced
convection regime, and should be able to model conjugate heat transfer between the fluid
and solid phases. A number of scalar species can be transported using the velocity
field determined from the fluid flow equations.
It can be used as a standalone application or can be included in
downstream applications interested in modeling multiphysics problems involving fluid flow.
!! system-scope-finish
!template-end!

!template! item key=system-purpose
!! system-purpose-begin
The purpose of this software is to allow simulation of fluid flow in regular and porous media.
It should be able to determine the pressure, velocity, and fluid temperature fields, as well
as the solid temperature field in the case of conjugate heat transfer simulations. It aims to also be able to
transport scalar species in the flow fields determined.
!! system-purpose-finish
!template-end!

!template! item key=assumptions-and-dependencies
The {{module}} module is designed with the fewest possible constraints on hardware and software.
For more context on this point, the {{module}} module SRS defers to the framework
[framework_srs.md#assumptions-and-dependencies]. The {{module}} module depends on the fluid
properties module for fluid properties.
All physics-based assumptions in this module's objects are highlighted in each object's respective
documentation pages. A summary of the key assumptions may be found on the {{module}} module [homepage](navier_stokes/index.md).
The key physics-based assumptions when modeling fluid flow consist of the compressibility,
from incompressible to fully compressible, the flow regime, whether turbulent or regular, and whether the media is
porous or not, which induces volumetric friction.
The key numerical decisions to be made when using the {{module}} module are whether to use a finite volume or finite element
discretization, and within those which numerical scheme to use, notably for discretizing the advective term.
!template-end!

!template! item key=reliability
The regression test suite will cover at least 77% of all lines of code within the {{module}}
module at all times. Known regressions will be recorded and tracked (see [#maintainability]) to an
independent and satisfactory resolution.
!template-end!

!template load file=sqa/module_srs.md.template category=porous_flow module=Porous Flow

!template! item key=system-scope
!! system-scope-begin
The Porous Flow module is a library of physics for fluid and heat flow in porous media. It is formulated
in an extremely general manner, so is capable of solving problems with an arbitrary number of phases
(gas, liquid, etc) and fluid components (species present in each fluid phase), using any set of primary
variables.

By simply adding pieces of physics together in an input file, the Porous Flow module enables the user
to model problems with any combination of fluid, heat and geomechanics.
!! system-scope-finish
!template-end!

!template! item key=system-purpose
!! system-purpose-begin
The purpose of the [!ac](MOOSE) Porous Flow module is to provide functionality to support simulations
for fluid and heat flow in porous media. The Porous Flow module can be easily coupled with other physics
modules in [!ac](MOOSE).
!! system-purpose-finish
!template-end!

!template! item key=assumptions-and-dependencies
The {{module}} module is designed with the fewest possible constraints on hardware and software.
For more context on this point, the {{module}} module SRS defers to the framework
[framework_srs.md#assumptions-and-dependencies].
!template-end!

!template! item key=reliability
The regression test suite will cover at least 95% of all lines of code within the {{module}}
module at all times. Known regressions will be recorded and tracked (see [#maintainability]) to an
independent and satisfactory resolution.
!template-end!

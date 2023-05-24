!template load file=sqa/module_srs.md.template category=porous_flow module=Porous Flow

!template! item key=system-scope
!! system-scope-begin
By simply adding pieces of physics together in an input file, the Porous Flow module enables the user
to model problems with any combination of fluid, heat, geomechanics and geochemistry.
!! system-scope-finish
!template-end!

!template! item key=system-purpose
!! system-purpose-begin
The purpose of the [!ac](MOOSE) Porous Flow module is to provide functionality to support simulations
for fluid and heat flow in porous media. Typical usage examples include groundwater flow, geothermal
modeling, geological storage of $\text{CO}_2$, and long-term mineralization due to geochemical reactions.
The Porous Flow module can be easily coupled with other physics modules in [!ac](MOOSE).
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

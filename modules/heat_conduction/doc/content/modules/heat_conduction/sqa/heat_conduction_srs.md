!template load file=sqa/module_srs.md.template category=heat_conduction module=Heat Conduction

!template! item key=system-purpose
!! system-purpose-begin
The [!ac](MOOSE) Heat Conduction module purpose is to model heat transfer due to
conduction and radiation.
!! system-purpose-finish
!template-end!

!template! item key=system-scope
!! system-scope-begin
The Heat Conduction module models volumetric heat transfer mechanisms due to
conduction and body sources/sinks. Additionally surface to surface conduction
and radiation may also be modeled.
!! system-scope-finish
!template-end!

!template! item key=assumptions-and-dependencies
The {{module}} module is designed with the fewest possible constraints on hardware and software.
For more context on this point, the {{module}} module SRS defers to the framework
[framework_srs.md#assumptions-and-dependencies] and ray tracing module
assumptions and dependencies. Any physics-based assumptions in
this module's objects are highlighted in their respective documentation pages.
!template-end!

!template! item key=reliability
The regression test suite will cover at least 87% of all lines of code within the {{module}}
module at all times. Known regressions will be recorded and tracked (see [#maintainability]) to an
independent and satisfactory resolution.
!template-end!

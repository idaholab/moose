!template load file=sqa/module_srs.md.template category=scalar_transport module=Scalar Transport

!template! item key=system-purpose
!! system-purpose-begin
The [!ac](MOOSE) Scalar Transport module purpose is to model mass transfer due to
advection, diffusion, and reaction.
!! system-purpose-finish
!template-end!

!template! item key=system-scope
!! system-scope-begin
The [!ac](MOOSE) Scalar Transport module scope is to model mass transfer due to
advection, diffusion, and reaction.
!! system-scope-finish
!template-end!

!template! item key=assumptions-and-dependencies
The {{module}} module is designed with the fewest possible constraints on hardware and software.
For more context on this point, the {{module}} module SRS defers to the framework
[framework_srs.md#assumptions-and-dependencies] assumptions and dependencies. Any physics-based assumptions in
this module's objects are highlighted in their respective documentation
pages.
!template-end!

!template! item key=reliability
The regression test suite will cover at least 90% of all lines of code within the {{module}}
module at all times. Known regressions will be recorded and tracked (see [#maintainability]) to an
independent and satisfactory resolution.
!template-end!

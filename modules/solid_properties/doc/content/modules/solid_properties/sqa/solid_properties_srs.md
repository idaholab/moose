!template load file=sqa/module_srs.md.template category=solid_properties module=Solid Properties

!template! item key=system-purpose
!! system-purpose-begin
The [!ac](MOOSE) Solid Properties module provides uniform interfaces to numerous physical properties
of solids and a library of solid objects based on these interfaces. This module is intended to be used by
a variety of MOOSE-based applications involving solid media.
!! system-purpose-finish
!template-end!

!template! item key=system-scope
!! system-scope-begin
The scope of the Solid Properties module includes properties of any solid media.
Additionally, the module includes some generic capability for defining solid
properties from user-defined functions.
!! system-scope-finish
!template-end!

!template! item key=assumptions-and-dependencies
The {{module}} module is designed with the fewest possible constraints on hardware and software.
For more context on this point, the {{module}} module SRS defers to the framework
[framework_srs.md#assumptions-and-dependencies]. Any physics-based assumptions in code simulations and
code objects are highlighted in their respective documentation pages.
!template-end!

!template! item key=reliability
The regression test suite will cover at least 84% of all lines of code within the {{module}}
module at all times. Known regressions will be recorded and tracked (see [#maintainability]) to an
independent and satisfactory resolution.
!template-end!

!template load file=sqa/module_srs.md.template category=reactor module=Reactor

!template! item key=system-purpose
!! system-purpose-begin
The [!ac](MOOSE) Reactor module provides an interface and libraries for creating meshes for
nuclear reactor systems.

It is intended to be used using other [!ac](MOOSE) ecosystem codes (including
MOOSE-wrapped applications) to perform the physics solve as it does not contain any physics.
Thus, the Reactor module uses the same object-oriented design
as [!ac](MOOSE) in order to make simulation design and new development straightforward for engineers
and researchers.
!! system-purpose-finish
!template-end!

!template! item key=system-scope
!! system-scope-begin
The scope of the Reactor module is to provide a set of utilities for building reactor meshes for finite element
or finite volume simulations. The modules relies on the [!ac](MOOSE) framework for utilities that are
common with other applications.

It covers several reactor types, mostly those with geometry that are axially extruded or close to
being so. This includes: liquid metal fast reactors, prismatic high temperature reactors, some variations of
micro reactors and pressurized water reactors.
Every design variation of each advanced reactor type cannot be covered by the module.
It covers the meshing and rotation of control drum in certain reactors.

The system contains, generally, a base set of mesh generators and meshing utilities.
Reactor module developers work with framework and other module and application developers to
ensure that the Reactor module provides adequate capability to support ongoing and prospective
research opportunities involving advanced reactors.
!! system-scope-finish
!template-end!

!template! item key=assumptions-and-dependencies
The Reactor module is designed with the fewest possible constraints on hardware and software.
For more context on this point, the Reactor module SRS defers to the framework
[framework_srs.md#assumptions-and-dependencies]. Any physics-based or geometry-based assumptions
in code simulations and code objects are highlighted in their respective documentation pages.
!template-end!

!template! item key=reliability
The regression test suite will cover at least 95% of all lines of code within the Reactor
module at all times. Known regressions will be recorded and tracked (see [#maintainability]) to an
independent and satisfactory resolution.
!template-end!

!template load file=sqa/module_srs.md.template module=Electromagnetics category=electromagnetics

!template! item key=system-purpose
!! system-purpose-begin
The [!ac](MOOSE) Electromagnetics module provides an interface to and library containing Maxwell's
equations within the [!ac](MOOSE) application ecosystem. It is intended to be used as either a standalone
simulation code for electrodynamics or coupled to other [!ac](MOOSE) ecosystem codes (including
MOOSE-wrapped applications). Thus, the Electromagnetics module uses the same object-oriented design
as [!ac](MOOSE) in order to make simulation design and new development straightforward for engineers
and researchers.
!! system-purpose-finish
!template-end!

!template! item key=system-scope
!! system-scope-begin
The scope of the Electromagnetics module is to provide a set of interfaces and objects for building
electrodynamics simulations based on the finite element method (FEM). Regarding solvers, meshing
libraries, as well as solution/coupling methods and interfaces, the Electromagnetics module relies on
the infrastructure provided by the [!ac](MOOSE) framework.

The system contains, generally, a base set of kernels, boundary conditions, and interface conditions
designed for the solution of vector fields derived from Maxwell's equations. Further, it currently
contains more specific capability in the following general areas:

- Wave reflection, transmission, and absorption
- Electrostatic contact on electrically imperfect surfaces

Electromagnetics module developers work with framework and other module and application developers to
ensure that the Electromagnetics module provides adequate capability to support on-going and prospective
research opportunities involving aspects of electromagnetics.
!! system-scope-finish
!template-end!

!template! item key=assumptions-and-dependencies
The Electromagnetics module is designed with the fewest possible constraints on hardware and software.
For more context on this point, the Electromagnetics module SRS defers to the framework
[framework_srs.md#assumptions-and-dependencies]. Any physics-based assumptions in code simulations and
code objects are highlighted in their respective documentation pages.
!template-end!

!template! item key=reliability
The regression test suite will cover at least 95% of all lines of code within the Electromagnetics
module at all times. Known regressions will be recorded and tracked (see [#maintainability]) to an
independent and satisfactory resolution.
!template-end!

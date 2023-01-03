!template load file=sqa/module_srs.md.template module=Reconstructed Discontinuous Galerkin category=rdg

!template! item key=system-purpose
!! system-purpose-begin
The MOOSE rDG module is a library for the implementation of simulation tools that solve
convection-dominated problems using the class of so-called reconstructed discontinuous Galerkin (rDG)
methods. The specific rDG method implemented in this module is rDG(P0P1), which is equivalent to the
second-order cell-centered finite volume method (FVM). Cell-centered FVMs are regarded as a subset of
rDG methods in the case when the baseline polynomial solution in each element is a constant
monomial. The FVMs are the most widely used numerical methods in areas such as computational fluid
dynamics (CFD) and heat transfer, computational acoustics, and magnetohydrodynamics (MHD).
!! system-purpose-finish
!template-end!

!template! item key=system-scope
!! system-scope-begin
The purpose of this software is to provide capability to MOOSE-based applications
to use a second-order, cell-centered finite volume method (FVM). This
module provides a systematic solution for implementing all required components in a second-order FVM
such as slope reconstruction, slope limiting, numerical flux, and proper boundary conditions.
Additionally, this module provides an implementation of these components for the
scalar advection equation.
!! system-scope-finish
!template-end!

!template! item key=assumptions-and-dependencies
The {{module}} module is designed with the fewest possible constraints on hardware and software.
For more context on this point, the {{module}} module SRS defers to the framework
[framework_srs.md#assumptions-and-dependencies]. Any physics-based or mathematics-based
assumptions in code simulations and code objects are highlighted in their
respective documentation pages.
!template-end!

!template! item key=reliability
The regression test suite will cover at least 65% of all lines of code within the {{module}}
module at all times. Known regressions will be recorded and tracked (see [#maintainability]) to an
independent and satisfactory resolution.
!template-end!

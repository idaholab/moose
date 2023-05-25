!template load file=sqa/module_srs.md.template category=level_set module=Level Set

!template! item key=system-scope
!! system-scope-begin
The [!ac](MOOSE) Level Set module provides capabilities for solving a set of level set equations. The level set method is commonly used for front tracking problems. The advection of the level set equation is solved using the Galerkin finite element method, and is stabilized with streamline Upwind/Petrov-Galerkin method. To preserve the smooth profile of the regularized level set variable, a re-initialization step can be used through the [!ac](MOOSE) multi-app system. The solutions are verified against classical benchmark problems.
!! system-scope-finish
!template-end!

!template! item key=system-purpose
!! system-purpose-begin
The purpose of the [!ac](MOOSE) Level Set module is to provides basic functionality to solve the level set equation. The Level Set module can be very easily coupled with other physics modules in [!ac](MOOSE).  The level set method has become popular in many disciplines, such as image processing, computer graphics, computational geometry, optimization, computational fluid dynamics, and computational biology.
!! system-purpose-finish
!template-end!

!template! item key=assumptions-and-dependencies
The {{module}} module is designed with the fewest possible constraints on hardware and software.
For more context on this point, the {{module}} module SRS defers to the framework
[framework_srs.md#assumptions-and-dependencies].
!template-end!

!template! item key=reliability
The regression test suite will cover at least 94% of all lines of code within the {{module}}
module at all times. Known regressions will be recorded and tracked (see [#maintainability]) to an
independent and satisfactory resolution.
!template-end!

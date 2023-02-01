!template load file=sqa/module_srs.md.template module=Ray Tracing category=ray_tracing

!template! item key=system-scope
!! system-scope-begin
The [!ac](MOOSE) Ray Tracing module provides tracing capabilities through a linear finite
element mesh, the ability to interact with or contribute to data local to the mesh during
the tracing and the ability to attach and propagate data along rays.
It can be used as a standalone application or can be included in downstream applications.
!! system-scope-finish
!template-end!

!template! item key=system-purpose
!! system-purpose-begin
The purpose of this software is to allow the integration and propagation of data along linear segments in
a curve.
It shall serve as the numerical backbone for Method of Characteristics and Monte Carlo transport solvers
derived from the [!ac](MOOSE) framework.
!! system-purpose-finish
!template-end!

!template! item key=assumptions-and-dependencies
The {{module}} module is designed with the fewest possible constraints on hardware and software.
For more context on this point, the {{module}} module SRS defers to the framework
[framework_srs.md#assumptions-and-dependencies] assumptions and
dependencies.
!template-end!

!template! item key=reliability
The regression test suite will cover at least 94% of all lines of code within the {{module}}
module at all times. Known regressions will be recorded and tracked (see [#maintainability]) to an
independent and satisfactory resolution.
!template-end!

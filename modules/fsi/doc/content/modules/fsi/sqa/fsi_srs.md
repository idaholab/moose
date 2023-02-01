!template load file=sqa/module_srs.md.template category=fsi module=Fluid Structure Interaction

!template! item key=system-scope
!! system-scope-begin
The [!ac](MOOSE) Fluid Structure Interaction module provides interface kernels
for simulating the interactions between neighboring fluid and solid
subdomains. It can be used as a standalone application or can be included in
downstream applications interested in modeling fluid structure interactions.
!! system-scope-finish
!template-end!

!template! item key=system-purpose
!! system-purpose-begin
The purpose of this software is to allow simulation of interfacial interactions between
fluid and solid domains. Both acoustic and full Navier-Stokes models of the
fluid domain are supported; small and finite strain models from the tensor
mechanics module may be used for the solid domain.
!! system-purpose-finish
!template-end!

!template! item key=assumptions-and-dependencies
The {{module}} module is designed with the fewest possible constraints on hardware and software.
For more context on this point, the {{module}} module SRS defers to the framework
[framework_srs.md#assumptions-and-dependencies] and [Navier-Stokes](navier_stokes_srs.md#assumptions-and-dependencies)
and [tensor mechanics](tensor_mechanics_srs.md#assumptions-and-dependencies) module assumptions and
dependencies. Any physics-based assumptions in this module's objects are highlighted in their respective
documentation pages.
!template-end!

!template! item key=reliability
The regression test suite will cover at least 90% of all lines of code within the {{module}}
module at all times. Known regressions will be recorded and tracked (see [#maintainability]) to an
independent and satisfactory resolution.
!template-end!

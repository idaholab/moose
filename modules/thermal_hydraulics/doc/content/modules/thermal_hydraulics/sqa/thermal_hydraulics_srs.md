!template load file=sqa/module_srs.md.template module=Thermal Hydraulics category=thermal_hydraulics

!template! item key=system-purpose
!! system-purpose-begin
The purpose of the MOOSE Thermal Hydraulics module is to provide capability for
performing system-level thermal hydraulic simulations in MOOSE. This capability
provides a convenient means of developing a system of connected components on
multiple domains, focused primarily on low-fidelity (one-dimensional and two-dimensional)
models. This allows large, complex systems, such as those present in reactor
systems, to be modeled without impractical computational resources.
!! system-purpose-finish
!template-end!

!template! item key=system-scope
!! system-scope-begin
The MOOSE Thermal Hydraulics module provides several additional systems, including
a component system, a closures system, and a control logic system. The module
includes basic components such as two-dimensional and three-dimensional heat
structures, which solve the transient heat conduction equation, along with components
that provide heat sources, boundary conditions, and interface conditions to these
components. The module also includes a suite of components for solving single-phase
flow, using numerical methods most suitable for compressible gas flows. These
single-phase flow components include flow channels, junctions, valves, walls,
and inlets/outlets. Additionally, the module provides turbomachinery components
such as a shaft, motor, compressor, and turbine. In addition to components,
the module provides basic closures for the single-phase flow model, as well
as control logic objects such as delays, trips, and PID controllers.
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
The regression test suite will cover at least 90% of all lines of code within the {{module}}
module at all times. Known regressions will be recorded and tracked (see [#maintainability]) to an
independent and satisfactory resolution.
!template-end!

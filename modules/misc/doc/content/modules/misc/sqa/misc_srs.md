!template load file=sqa/module_srs.md.template category=misc module=Misc

!template! item key=system-scope
!! system-scope-begin
The [!ac](MOOSE) Misc module is designed only to provide code modules that support the solution of physics models that are primarily defined by other MOOSE modules and MOOSE-based applications.

The Misc module provides several specializations of MOOSE classes that are used to support calculations performed in a variety of physics calculations. These include `Kernel`, `AuxKernel`, `Material` and `Postprocessor` classes that computing terms in diffusion equations, compute density, internal volume, and perform other similar functions.
!! system-scope-finish
!template-end!

!template! item key=system-purpose
!! system-purpose-begin
The purpose of the [!ac](MOOSE) {{module}} module is to provide a set of models that support a variety of simulations performed using other MOOSE modules or MOOSE-based applications.
!! system-purpose-finish
!template-end!

!template! item key=assumptions-and-dependencies
The {{module}} module has no constraints on hardware and software beyond those of the [MOOSE framework](framework_srs.md#assumptions-and-dependencies).
The {{module}} module provides access to a number of code objects that perform various computations. These objects each make their own physics-based assumptions, such as the units of the inputs and outputs. Those assumptions are described in the documentation for those individual objects.
!template-end!

!template! item key=reliability
The regression test suite will cover at least 55% of all lines of code within the {{module}}
module at all times. Known regressions will be recorded and tracked (see [#maintainability]) to an
independent and satisfactory resolution.
!template-end!

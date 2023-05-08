!template load file=sqa/module_sdd.md.template category=misc module=Misc

!template! item key=introduction
The [!ac](MOOSE) {{module}} module provides a set of objects that are used to support a variety of simulations performed using other MOOSE modules or MOOSE-based applications. This module relies on MOOSE for solving the systems of equations that are primarily defined by code outside this module, and simply provides commonly-used code objects that can support those calculations. This document describes the system design of the {{module}} module.
!template-end!

!template! item key=system-scope
!include misc_srs.md start=system-scope-begin end=system-scope-finish
!template-end!

!template! item key=dependencies-and-limitations
The [!ac](MOOSE) {{module}} module inherits the [software dependencies of the MOOSE framework](framework_sdd.md#dependencies-and-limitations), with no additional dependencies.
!template-end!

!template! item key=design-stakeholders
!include framework_sdd.md start=design-stakeholders-begin end=design-stakeholders-finish
!template-end!

!template! item key=system-design
The [!ac](MOOSE) {{module}} module relies on MOOSE to solve the governing equations for the physics models that it supports. The design of MOOSE is based on the concept of modular code objects that define all of the aspects of the physics model. This module follows this design, providing code objects that define specific aspects of the solutions for its physics that derive from the base classes defined by the MOOSE framework and the modules that it depends on.

The {{module}} module provides several specializations of MOOSE classes that are used to support calculations performed in a variety of physics calculations. These include `Kernel`, `AuxKernel`, `Material` and `Postprocessor` classes that computing terms in diffusion equations, compute density, internal volume, and perform other similar functions.
!template-end!

!template! item key=system-structure
The [!ac](MOOSE) {{module}} module relies on the MOOSE framework to provide the core functionality of solving multiphysics problems using the finite element method. The structure of the {{module}} module is based on defining C++ classes that derive from base classes in the MOOSE framework to provide functionality for solution of a variety of physics problems. By using the interfaces defined in MOOSE for these classes, this module is able to rely on MOOSE to execute these models at the appropriate times during the simulation and use their results in the desired ways.
!template-end!

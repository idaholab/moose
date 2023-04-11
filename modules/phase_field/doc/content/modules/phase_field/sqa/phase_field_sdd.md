!template load file=sqa/module_sdd.md.template category=phase_field module=Phase Field

!template! item key=introduction
Computing microstructure evolution of materials is important for a variety of applications. These can employ a variety of assumptions of material behavior, and can be either run as standalone single-physics problems or couple the evolution with that of other physics. The [!ac](MOOSE) {{module}} module provides a foundational set of models for modeling microstructure evolution. This module relies on MOOSE for solving its system of equations, and is designed to be readily extended or coupled with other physics models. This document describes the system design of the {{module}} module.
!template-end!

!template! item key=system-scope
!include phase_field_srs.md start=system-scope-begin end=system-scope-finish
!template-end!

!template! item key=dependencies-and-limitations
The [!ac](MOOSE) {{module}} module inherits the [software dependencies of the MOOSE framework](framework_sdd.md#dependencies-and-limitations), with no additional dependencies.
!template-end!

!template! item key=design-stakeholders
!include framework_sdd.md start=design-stakeholders-begin end=design-stakeholders-finish
!template-end!

!template! item key=system-design
TODO: Put phase_field-specific text here
The [!ac](MOOSE) {{module}} module relies on MOOSE to solve the governing equations for teh phase field method, which can include contributions from the thermodynamic potentials provided by this module, elastic energy contributions from mechanical deformation, and temperature fields solved using teh heat conduction module. The design of MOOSE is based on the concept of modular code objects that define all of the aspects of the physics model. This module follows this design, providing code objects that define specific aspects of the solutions for its physics that derive from the base classes defined by the MOOSE framework and the modules that it depends on.

The {{module}} module provides specialized `Kernel` classes that compute the contributions from the terms in the partial differential equations governing the evolution of phase fields. In addition, it provides `IC`, `BC`, `Action`, `Marker`, `MeshGenerator`, `Function`, `AuxKernel`, `Postprocessor` and `VectorPostprocessor` classes to facilitate various aspects of these simulations.
!template-end!

!template! item key=system-structure
The [!ac](MOOSE) {{module}} module relies on the MOOSE framework to provide the core functionality of solving multiphysics problems using the finite element method. The structure of the {{module}} module is based on defining C++ classes that derive from classes in the MOOSE framework to provide functionality for solution of phase field problems. By using the interfaces defined in MOOSE base classes for these classes, this module is able to rely on MOOSE to execute these models at the appropriate times during the simulation and use their results in the desired ways.
!template-end!

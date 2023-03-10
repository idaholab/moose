!template load file=sqa/module_sdd.md.template category=contact module=Contact

!template! item key=introduction
In simulations of deformable bodies, it is often important to impose constraints on the solutions to ensure that the bodies do not interpenetrate, and to appropriately represent the physical behavior of the interacting surfaces. The [!ac](MOOSE) {{module}} module provides the capabilities to enforce these mechanical constraints in simulations of deformable bodies. This module relies on MOOSE for solving its system of equations, and works in conjunction with models for the deformation of those bodies provided by the Tensor Mechanics module.  This document describes the system design of the {{module}} module.
!template-end!

!template! item key=system-scope
!include contact_srs.md start=system-scope-begin end=system-scope-finish
!template-end!

!template! item key=dependencies-and-limitations
The [!ac](MOOSE) {{module}} module inherits the software dependencies of the [MOOSE framework](framework_sdd.md#dependencies-and-limitations) and [Tensor Mechanics module](tensor_mechanics_sdd.md#dependencies-and-limitations) with no additional dependencies.
!template-end!

!template! item key=design-stakeholders
!include framework_sdd.md start=design-stakeholders-begin end=design-stakeholders-finish
!template-end!

!template! item key=system-design
The [!ac](MOOSE) {{module}} module relies on MOOSE to solve the equilibrium equations for mechanical deformation, taking into account the additional conditions imposed for mechanical contact enforcement. The design of MOOSE is based on the concept of modular code objects that define all of the aspects of the physics model. This module follows this design, providing code objects that define specific aspects of the solutions for its physics that derive from the base classes defined by the MOOSE framework and the modules that it depends on.

The {{module}} module provides specialized `Constraint` classes that perform the majority of the effort involved in contact enforcement. This module is designed to work in conjunction with models provided outside this module, typically by the [Tensor Mechanics](tensor_mechanics/index.md) module, that solve the equilibrium equations for mechanical deformation of the bodies, and augments those solutions with the terms due to contact.  It also provides other `Action`, `AuxKernel`, `Damper`, `LineSearch`, `Postprocessor`, `Problem`, `Split`, and `UserObject` classes to facilitate various aspects of these simulations.
!template-end!

!template! item key=system-structure
The [!ac](MOOSE) {{module}} module relies on the MOOSE framework to provide the core functionality of solving multiphysics problems using the finite element method, and on the Tensor Mechanics module to provide the models for mechanical deformation of the interacting bodies. The structure of the {{module}} module is based on defining C++ classes that derive from classes in the MOOSE framework to provide functionality for mechanical contact enforcement. By using the interfaces defined in MOOSE base classes for these classes, this module is able to rely on MOOSE to execute these models at the appropriate times during the simulation and use their results in the desired ways.
!template-end!

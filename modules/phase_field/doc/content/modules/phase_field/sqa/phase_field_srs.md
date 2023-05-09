!template load file=sqa/module_srs.md.template category=phase_field module=Phase Field

!template! item key=system-scope
!! system-scope-begin
The [!ac](MOOSE) Phase Field module provides an extensible set of capabilities for solving for microstructual evolution of multi-phase and multi-componeht systems using conserved and non-conserved order parameters. It provides a set of C++ classes that define interfaces for MOOSE `Kernel` objects that implement various common phase field formulations including polycrystalline grain growth models. These models support one-, two-, and three-dimensional simulation domains.

For modeling nucleation events the module includes a discrete nucleation system that supports nucleation based on order parameter pinning and artificial driving force insertion which enable nucleation events in conserved and non-conserved order parameter fields. Support is provided for physics informed timestepping and mesh refinement.

Utilities for free energy extraction from thermodynamic databases are provided, as well as a phase field model that supports sublattice concentration models found in thermodynamic databases.

The module also comprises a set of initial conditions to set up common microstructures used in phase field modeling.
!! system-scope-finish
!template-end!

!template! item key=system-purpose
!! system-purpose-begin
The purpose of the [!ac](MOOSE) Phase Field module is to provide the foundational capabilities for phase field modeling of microstructure evolution. It implements common single- and multi-phase phase field formalisms and enables the modeling of multicomponent systems. It is intended to both provide a basic set of capabilities and also be readily extensible by applications based on it to represent specialized material behavior.
!! system-purpose-finish
!template-end!

!template! item key=assumptions-and-dependencies
The {{module}} module has no constraints on hardware and software beyond those of the [MOOSE framework](framework_srs.md#assumptions-and-dependencies).
The {{module}} module provides access to a number of code objects that perform computations such as material behavior and boundary conditions. These objects each make their own physics-based assumptions, such as the units of the inputs and outputs. Those assumptions are described in the documentation for those individual objects.
!template-end!

!template! item key=reliability
The regression test suite will cover at least 89% of all lines of code within the {{module}}
module at all times. Known regressions will be recorded and tracked (see [#maintainability]) to an
independent and satisfactory resolution.
!template-end!

!template load file=sqa/module_srs.md.template category=tensor_mechanics module=Tensor Mechanics

!template! item key=system-scope
!! system-scope-begin
The [!ac](MOOSE) Tensor Mechanics module provides an extensible set of capabilities for solving for mechanical deformation of solids and structures. It provides a set of C++ base classes that define interfaces for MOOSE `Material` objects that compute various mechanical behavior of materials at quadrature points, which include elastic properties, strains, eigenstrains, stresses, inelastic behavior, and damage. It also provides the needed `Kernel` classes to account for the contributions of the stress, inertia and damping in the solution for the displacement field that satisfies equilibrium. These models support one-, two-, and three-dimensional models of continuous materials, with variety of options for the treatment of the lower-dimensional models, including plane stress, plane strain, axisymmetry, and generalized plane strain. These models support both small- and finite-strain assumptions.

For modeling discrete interfaces between solid elements, this module provides a system similar to that for continuous materials that permits the definition of traction-separation laws for cohesive behavior. It also has similar support for lower-dimensional elements to represent structural elements such as beams and shells. As for the continuum models, sets of `Material` and `Kernel` classes are defined for these cases.

This module also provides a comprehensive set of boundary conditions relevant for mechanics modeling, such as pressure and traction boundary conditions. It also provides extensive postprocessing capabilities for computing quantities relevant to mechanics, such as fracture integrals.

In addition to defining the base classes that enable modeling arbitrary materials, this module also provides a set of specializations of those models for widely-used assumptions of material behavior. These include elasticity tensors defined in a variety of ways for isotropic and anisotropic materials, basic creep, plasticity and damage models, and models for eigenstrains due to thermal expansion.
!! system-scope-finish
!template-end!

!template! item key=system-purpose
!! system-purpose-begin
The purpose of the [!ac](MOOSE) Tensor Mechanics module is to provide the foundational capabilities for computing mechanical deformation of solids and structures. It computes strains and stresses, and solves for the displacement field that satisfies equlibrium. It is intended to both provide a basic set of capabilities and also be readily extensible by applications based on it to represent specialized material behavior.
!! system-purpose-finish
!template-end!

!template! item key=assumptions-and-dependencies
The {{module}} module has no constraints on hardware and software beyond those of the [MOOSE framework](framework_srs.md#assumptions-and-dependencies).
The {{module}} module provides access to a number of code objects that perform computations such as material behavior and boundary conditions. These objects each make their own physics-based assumptions, such as the units of the inputs and outputs. Those assumptions are described in the documentation for those individual objects.
!template-end!

!template! item key=reliability
The regression test suite will cover at least 86% of all lines of code within the {{module}}
module at all times. Known regressions will be recorded and tracked (see [#maintainability]) to an
independent and satisfactory resolution.
!template-end!

!template load file=sqa/module_srs.md.template category=xfem module=XFEM

!template! item key=system-scope
!! system-scope-begin
The [!ac](MOOSE) XFEM module provides capabilities for representing mesh-independent discontinuities in a finite-element model. This is done using the phantom-node variant of the extended finite element method, which introduces discontinuities by duplicating elements cut by the discontinuity, re-connecting those elements to the mesh in an appropriate way, and modifying the integration rule of these cut elements to reflect the fact that only part of the elements represent physical material.

This module provides all of the capabilities needed to use phantom-node-based XFEM. The core of this capability is the cutting algorithm, known as the element fragment algorithm (EFA). The EFA determines which elements should be deleted and added, and which nodes should be added. The XFEM module provides all the capabilities needed to store and use data about the elements that have been cut, and to modify the integration rules for those cut elements.

The XFEM module also provides capabilities for the user to define how the mesh is cut in a variety of ways, including based on primitive geometric objects, meshes, level set fields, or paths determined by stresses or fracture integrals. For crack propagation based on fracture integrals, the XFEM module can provide the information on crack geometry needed to compute those integrals, which are computed externally to this module. 

In addition, the XFEM module contains tools to support the setup of models that use XFEM and outputting relevant data for use in visualizing results.
!! system-scope-finish
!template-end!

!template! item key=system-purpose
!! system-purpose-begin
The purpose of the [!ac](MOOSE) XFEM module is to allow for mesh-independent discontinuities to be introduced into finite-element models using the extended finite element method. These discontinuities can be due to a variety of phenomena, including fractures and material interfaces, and can be used to represent the boundary of the domain being modeled.
!! system-purpose-finish
!template-end!

!template! item key=assumptions-and-dependencies
The {{module}} module has no constraints on hardware and software beyond those of the [MOOSE framework](framework_srs.md#assumptions-and-dependencies) and the [Tensor Mechanics](tensor_mechanics_srs.md#assumptions-and-dependencies) module.
The {{module}} module provides access to a number of code objects that perform computations. These objects each make their own physics-based assumptions, such as the units of the inputs and outputs. Those assumptions are described in the documentation for those individual objects.
!template-end!

!template! item key=reliability
The regression test suite will cover at least 80% of all lines of code within the {{module}}
module at all times. Known regressions will be recorded and tracked (see [#maintainability]) to an
independent and satisfactory resolution.
!template-end!

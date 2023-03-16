!template load file=sqa/module_sdd.md.template category=xfem module=XFEM

!template! item key=introduction
The physics models in [!ac](MOOSE) typically represent spatially continuous behavior, for which the finite element method is well suited. However, discontinuities can arise in the physical systems being modeled, and representing those discontinuities can be challenging, especially when they move or grow, as is the case for moving material interfaces or propagating cracks.  The [!ac](MOOSE) {{module}} module provides capabilities to represent arbitrary, mesh-independent discontinuities in physical solutions using the extended finite element method. This module relies on MOOSE for solving its system of equations, and is designed to be flexible in its ability to introduce discontinuities to arbitrary physics models. This document describes the system design of the {{module}} module.
!template-end!

!template! item key=system-scope
!include xfem_srs.md start=system-scope-begin end=system-scope-finish
!template-end!

!template! item key=dependencies-and-limitations
The [!ac](MOOSE) {{module}} module inherits the software dependencies of the [MOOSE framework](framework_sdd.md#dependencies-and-limitations) and [Tensor Mechanics module](tensor_mechanics_sdd.md#dependencies-and-limitations) with no additional dependencies.
!template-end!

!template! item key=design-stakeholders
!include framework_sdd.md start=design-stakeholders-begin end=design-stakeholders-finish
!template-end!

!template! item key=system-design
The [!ac](MOOSE) {{module}} module relies on MOOSE to solve the governing equations for arbitrary physics, and provides the additional capabilities needed to introduce spatial discontinuities in those physics models. Discontinuities are introduced using the phantom-node variant of the extended finite element method, which duplicates elements cut by the discontinuity, re-connects those elements to the mesh in an appropriate way, and modifies the integration rule of these cut elements to reflect the fact that only part of the elements represent physical material.
   
The core of this capability is the cutting algorithm, known as the element fragment algorithm (EFA). The EFA determines which elements should be deleted and added, and which nodes should be added. The EFA is designed to have minimal dependencies on MOOSE, and uses its own simple mesh data structures. The EFA mesh is defined using information provided to it about the elements that make up the MOOSE finite element mesh used for the physics solution. Once the EFA identifies the changes to be made to the mesh, the XFEM module modifies the MOOSE mesh, and uses its own data structures to store data about the cut locations that is necessary for integration of the fields that are nonzero over only a portion of the element.

For integration of the fields on the cut elements, the XFEM module uses a moment-fitting method, which uses the original quadrature points, but modifies their weights using values that are obtained through a least-squares procedure. It also provides an option to simply multiply the original quadrature-point weights by the volume fraction of the cut element that represents physical material, which is a special case of the moment-fitting method.

The XFEM module provides the capability for the user to define how the mesh is cut in a variety of ways, including based on primitive geometric objects, meshes, level set fields, or paths determined by stresses or fracture integrals. A base class, based on the MOOSE `ElementUserObject`, is provided in the XFEM module, which provides a way for cut elements to be identified and have those elements and the cut locations be conveyed to the EFA. A number of objects that derive from that base cut definition class are provided to define cuts in a variety of ways. For crack propagation based on fracture integrals, the XFEM module can provide the information on crack geometry needed to compute those integrals to the [Tensor Mechanics](tensor_mechanics/index.md) module, which computes those integrals. If those fracture integrals indicate that a crack should be extended, the cut definition is modified to reflect those changes.
   
The design of MOOSE is based on the concept of modular code objects that define all of the aspects of the physics model. Although there are specialized capabiliites in XFEM that fall outside the standard MOOSE base classes, whereever it is possible, this module follows this design, providing code objects that define specific aspects of the solutions for its physics that derive from the base classes defined by the MOOSE framework and the modules that it depends on.
!template-end!

!template! item key=system-structure
The [!ac](MOOSE) {{module}} module relies on the MOOSE framework to provide the core functionality of solving multiphysics problems using the finite element method, and on other physics modules to solve for the continuous solutions that are enriched using the extended finite element method. Although the XFEM module is designed to operate with arbitrary physics, there are models specific to fracture propagation that are designed to operate specifically with the [Tensor Mechanics](tensor_mechanics/index.md) module. The structure of the {{module}} module is based on defining C++ classes that derive from classes in the MOOSE framework to provide needed functionality wherever possible. By using the interfaces defined in MOOSE base classes for these classes, this module is able to rely on MOOSE to execute these models at the appropriate times during the simulation and use their results in the desired ways.
!template-end!

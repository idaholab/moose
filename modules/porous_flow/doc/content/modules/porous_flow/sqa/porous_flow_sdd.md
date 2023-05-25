!template load file=sqa/module_sdd.md.template category=porous_flow module=Porous Flow

!template! item key=introduction
The [!ac](MOOSE) {{module}} module is based on the MOOSE framework and thus inherits
the unique features and base characteristics of the framework, as outlined in the [framework_sdd.md].
Specific details unique to the module are outlined in this document.
!template-end!

!template! item key=system-scope
!include porous_flow_srs.md start=system-scope-begin end=system-scope-finish
!template-end!

!template! item key=dependencies-and-limitations
The {{module}} module inherits the [software dependencies and limitations of the MOOSE framework](framework_sdd.md#dependencies-and-limitations).
It depends on the [Fluid Properties](fluid_properties/index.md) module for all fluid property calculations,
the [Tensor Mechanics](tensor_mechanics/index.md) module for the mechanical aspects of coupled flow
and geomechanics models, and the [Geochemistry](geochemistry/index.md optional=True) module for coupled flow and
geochemical reactions. The {{module}} module currently has the following limitations in functionality:

- As it depends on other physics modules for fluid properties, mechanics and geochemistry calculations,
  the {{module}} module is limited to the capability of each of the physics modules listed above. For
  example, the only fluids that can be used are those that are made available in the [Fluid Properties](fluid_properties/index.md)
  module.
- Only a small selection of constitutive models for properties like relative permeability or
  porosity-permeability relationships are available to the user. If a user requires some other
  constitutive model, they must develop that functionality themselves.
- The {{module}} module was developed before the inclusion of Automatic Differentiation (AD) capability
  in MOOSE, and the physics kernels are not currently designed for AD materials, meaning that new
  functionality currently requires hand-coded Jacobian entries.
!template-end!

!template! item key=design-stakeholders
!include framework_sdd.md start=design-stakeholders-begin end=design-stakeholders-finish
!template-end!

!template! item key=system-design
The {{module}} module inherits the wide range of pluggable systems from MOOSE. More
information regarding MOOSE system design can be found in the framework [framework_sdd.md#system-design]
section.

The {{module}} module provides functionality to support simulations for fluid and heat flow in porous
media. More detailed information on the theory of the module can be found on the [module home page](porous_flow/index.md)
under the subsection "Module overview". Numerical implementation details for the module can be found
on the same page, under the subsection "Implementation details". A requirement of each {{module}}
simulation is the [`PorousFlowDictator`](porous_flow/dictator.md) object, which holds information about
the nonlinear solution variables within the module, as well as the number of fluid phases and fluid
components in each simulation. This object ensures that all of the required derivative terms are
included in the Jacobian to aid solver convergence.

Kernels for a wide range of transport physics, such as advection, diffusion, hydrodynamic dispersion,
and heat conduction are provided, as well as Kernels that couple fluid and heat flow to geomechanics
and geochemistry. These are listed in the [governing equations](porous_flow/governing_equations.md)
alongside the terms of the governing equations that each kernel represents.

Various [boundary conditions](porous_flow/boundaries.md) are provided to represent common cases, such
as boundaries to represent large aquifers, or boundaries to model evapotranspiration at a surface.
Similarly, a number of [Dirac kernels](porous_flow/sinks.md) objects are provided, from simple point
sources to [wellbores](dirackernels/PorousFlowPeacemanBorehole.md).

The {{module}} module also provides several choices of [constitutive equations](porous_flow/material_laws.md)
for relative permeability, capillary pressure, porosity, and permeability etc. Saturation history
dependent [hysteresis](porous_flow/hysteresis.md) in capillary pressure and relative permeability is
also included.

Documentation for each object, data structure, and process specific to the
module are kept up-to-date alongside the MOOSE documentation. Expected failure
modes and error conditions are accounted for via regression testing, and error
conditions are noted in object documentation where applicable.
!template-end!

!template! item key=system-structure
The architecture of the {{module}} module consists of a core and several pluggable systems (both
inherited from the MOOSE framework). The core of MOOSE consists of a number of key objects responsible
for setting up and managing the user-defined objects of a finite element or
finite volume simulation. This core set of objects has limited extendability and exists for every
simulation configuration that the module is capable of running.

!syntax complete subsystems=False actions=False objects=False groups=PorousFlowApp

!style halign=left
The MooseApp is the top-level object used to hold all of the other objects in a simulation. In a
normal simulation a single MooseApp object is created and "run()". This object uses its Factory
objects to build user-defined objects which are stored in a series of Warehouse objects and
executed. The Finite Element and/or Finite Volume data is stored in the Systems and Assembly objects while the domain
information (the Mesh) is stored in the Mesh object. A series of threaded loops are used to run
parallel calculations on the objects created and stored within the warehouses.

MOOSE's pluggable systems are documented on [MOOSE website](https://mooseframework.inl.gov), and those
for the {{module}} module are on this webpage, accessible through the high-level system links above.
Each of these systems has a set of defined polymorphic interfaces and are designed to accomplish a
specific task within the simulation. The design of these systems is fluid and is managed through agile
methods and ticket request system either on GitHub (for MOOSE) or on the defined software repository
for this application.
!template-end!

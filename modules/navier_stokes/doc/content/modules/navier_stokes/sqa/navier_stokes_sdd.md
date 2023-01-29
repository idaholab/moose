!template load file=sqa/module_sdd.md.template category=navier_stokes module=Navier Stokes

!template! item key=introduction
The [!ac](MOOSE) {{module}} module is based on the MOOSE framework and thus inherits
the unique features and base characteristics of the framework, as outlined in the [framework_sdd.md].
Specific details unique to the module are outlined in this document.
!template-end!

!template! item key=system-scope
!include navier_stokes_srs.md start=system-scope-begin end=system-scope-finish
!template-end!

!template! item key=dependencies-and-limitations
The {{module}} module inherits the
[software dependencies and limitations of the MOOSE framework](framework_sdd.md#dependencies-and-limitations),
as well as the dependencies and limitations of the [Heat Conduction](heat_conduction_sdd.md#dependencies-and-limitations) module
when performing coupled heat transfer simulations, the [Fluid Properties](tensor_mechanics_sdd.md#dependencies-and-limitations optional=True) module
when using the specific fluid properties in this module, and the [rDG](rdg_sdd.md#dependencies-and-limitations optional=True) module
when using a discretization from the reconstructed Discontinuous Galerkin family.

While the {{module}} module has received significant development and numerous studies were performed with its service,
the diversity of the flow problems that a modeler may encounter is so large that it may not have all the features desired by potential users.
There is current programmatic funding at the Idaho National Laboratory and Argonne National Laboratory to support
development of the {{module}} module.

Notable limitations include the assumptions of the turbulence model chosen, the lack of anisotropic turbulence models, of
automated wall treatment, of automated treatment of the transition regime between turbulent and laminar flow and
the lack of support for multiphase fluid flow. Each numerical discretization is also not feature complete,
which means that some turbulence/porous/other models are only available for some numerical schemes. The {{module}} module [homepage](navier_stokes/index.md)
summarizes the extent of support for common models in each discretization.
!template-end!

!template! item key=design-stakeholders
!include framework_sdd.md start=design-stakeholders-begin end=design-stakeholders-finish
!template-end!

!template! item key=system-design
The {{module}} module inherits the wide range of pluggable systems from MOOSE. More
information regarding MOOSE system design can be found in the framework [framework_sdd.md#system-design]
section.

The {{module}} module is designed to handle a wide variety of flow regimes, as well as both regular and porous media flow.
This can only be addressed with different discretizations of the flow equations, with appropriate kernels and boundary conditions.
The finite element and finite volume discretizations are also performed by different objects. The plurality of
objects can be challenging to users and developers. On the user side, the prefix of the name of each kernel/boundary condition/user object
in the module indicates the discretization it is valid for. On the developer side, the main complexities in the modeling were
as often as possible concentrated in the base classes, with the derived classes adapting the object to the discretization at hand.
In some cases, discretizations were implemented as special cases of more general ones. For example, incompressible flow in porous
media is a specialization of non-porous weakly compressible flow with a constant density and a non-unity porosity.

The {{module}} module is faced with the unique challenge that two variable sets, primitive and conservative,
are commonly used for different flow regimes and some of these flow regimes are of interest within the scope of the module.
The module is designed to be able to handle both, with utilities to perform conversions from one to the
other as appropriate. The module is not currently designed to handle transitions between the two variable sets at a fluid interface.

Each major class of discretizations has its own index page, notably the [continuous Galerkin finite element](navier_stokes/cgfe.md),
the [incompressible finite volume](navier_stokes/insfv.md), the [weakly compressible finite volume](navier_stokes/wcnsfv.md)
and the [incompressible finite volume porous media](navier_stokes/pinsfv.md) discretizations.
Documentation for each object, data structure, and process specific to the
module are kept up-to-date alongside the MOOSE documentation. Expected failure
modes and error conditions are accounted for via regression testing, and error
conditions are noted in object documentation where applicable.
!template-end!

!template! item key=system-structure
The architecture of the {{module}} module consists of a core and several pluggable systems (both
inherited from the MOOSE framework). The core of MOOSE consists of a number of key objects responsible
for setting up and managing the user-defined objects of a finite element or
finite volume simulation. This core set of
objects has limited extendability and exists for every simulation configuration that the module is
capable of running.

!syntax complete subsystems=False actions=False objects=False groups=NavierStokesApp

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

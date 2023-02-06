!template load file=sqa/module_sdd.md.template module=Optimization category=optimization

!template! item key=introduction
The [!ac](MOOSE) {{module}} module is based on the MOOSE framework and thus inherits
the unique features and base characteristics of the framework, as outlined in the [framework_sdd.md].
Specific details unique to the module are outlined in this document.
!template-end!

!template! item key=system-scope
!include optimization_srs.md start=system-scope-begin end=system-scope-finish
!template-end!

!template! item key=dependencies-and-limitations
The {{module}} module inherits the
[software dependencies of the MOOSE framework](framework_sdd.md#dependencies-and-limitations),
with no additional dependencies. The module relies heavily on the TAO optimization package, which is available through the framework's inclusion of PETSc. The design of this module is motivated by the
needs of its client applications.
!template-end!

!template! item key=design-stakeholders
!include framework_sdd.md start=design-stakeholders-begin end=design-stakeholders-finish
!template-end!

!template! item key=system-design
The {{module}} module inherits the wide range of pluggable systems from MOOSE.
More information regarding MOOSE system design can be found in the framework
[framework_sdd.md#system-design] section. Most of these inherited systems are
utilized to build physics models for which optimization is being
performed. Other than these, the [MultiApps](MultiApps/index.md),
[Transfers](Transfers/index.md), and [Reporters](Reporters/index.md) systems
in the framework are utilized to execute the physical model, manipulate
optimization data, and define the optimization objective and gradient.
Documentation for each object, data structure, and process specific to the
module are kept up-to-date alongside the MOOSE documentation. Expected failure
modes and error conditions are accounted for via regression testing, and error
conditions are noted in object documentation where applicable.
!template-end!

!template! item key=system-structure
The architecture of the {{module}} module consists of a core and several pluggable systems.
The core of MOOSE consists of a number of key objects responsible for setting up and managing the
user-defined objects of a finite element simulation. This core set of objects has limited
extendability and exists for every simulation configuration that the module is capable of running.

!syntax complete subsystems=False actions=False objects=False groups=StochasticToolsApp

!style halign=left
The MooseApp is the top-level object used to hold all of the other objects in a simulation. In a
normal simulation, a single MooseApp object is created and "run()". This object uses its Factory
objects to build user-defined objects which are stored in a series of Warehouse objects and
executed. The Finite Element data is stored in the Systems and Assembly object while the domain
information (the Mesh) is stored in the Mesh object. A series of threaded loops are used to run
parallel calculations on the objects created and stored within the warehouses.

MOOSE's pluggable systems are documented on the [MOOSE website](https://mooseframework.inl.gov), and those
for the {{module}} module are on this webpage, accessible through the high-level system links above.
Each of these systems has a set of defined polymorphic interfaces and are designed to accomplish a
specific task within the simulation. The design of these systems is solid and is managed through agile
methods and ticket request system on the [MOOSE GitHub repository](https://github.com/idaholab/moose).
!template-end!

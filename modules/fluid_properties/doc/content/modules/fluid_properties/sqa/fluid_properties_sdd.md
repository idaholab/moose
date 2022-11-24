!template load file=sqa/module_sdd.md.template category=fluid_properties module=Fluid Properties

!template! item key=introduction
The [!ac](MOOSE) {{module}} module is based on the MOOSE framework and thus inherits
the unique features and base characteristics of the framework, as outlined in the [framework_sdd.md].
Specific details unique to the module are outlined in this document.
!template-end!

!template! item key=system-scope
The purpose of this software is to provide physical properties of fluids to
MOOSE-based applications, suitable for a variety of fluid formulations.
Specifically, the following objectives are sought:

- Provide uniform interfaces for accessing single-phase and two-phase fluid properties
- Provide a library of fluid properties
!template-end!

!template! item key=dependencies-and-limitations
The {{module}} module inherits the
[software dependencies of the MOOSE framework](framework_sdd.md#dependencies-and-limitations),
with no additional dependencies. The design of this module is already
relatively stable; the majority of additional contributions aim to extend the
list of fluid properties available and make various feature improvements. The
design is not expected to be impacted significantly by factors such as budget,
schedule, and staffing.
!template-end!

!template! item key=design-stakeholders
!include framework_sdd.md start=design-stakeholders-begin end=design-stakeholders-finish
!template-end!

!template! item key=system-design
The {{module}} module inherits the wide range of pluggable systems from MOOSE. More
information regarding MOOSE system design can be found in the framework [framework_sdd.md#system-design]
section. The {{module}} module does not provide any physical models for fluid
dynamics, only models for computing various physical properties of fluids.
Documentation for each object, data structure, and process specific to the
module are kept up-to-date alongside the MOOSE documentation. Expected failure
modes and error conditions are accounted for via regression testing, and error
conditions are noted in object documentation where applicable.
!template-end!

!template! item key=system-structure
The architecture of the {{module}} module consists of a core and several pluggable systems (both
inherited from the MOOSE framework). The core of MOOSE consists of a number of key objects responsible
for setting up and managing the user-defined objects of a finite element simulation. This core set of
objects has limited extendability and exists for every simulation configuration that the module is
capable of running.

!syntax complete subsystems=False actions=False objects=False groups=FluidPropertiesApp

!style halign=left
The MooseApp is the top-level object used to hold all of the other objects in a simulation. In a
normal simulation a single MooseApp object is created and "run()". This object uses its Factory
objects to build user defined objects which are stored in a series of Warehouse objects and
executed. The Finite Element data is stored in the Systems and Assembly object while the domain
information (the Mesh) is stored in the Mesh object. A series of threaded loops are used to run
parallel calculations on the objects created and stored within the warehouses.

MOOSE's pluggable systems are documented on [MOOSE website](https://mooseframework.inl.gov), and those
for the {{module}} module are on this webpage, accessible through the high-level system links above.
Each of these systems has a set of defined polymorphic interfaces and are designed to accomplish a
specific task within the simulation. The design of these systems is fluid and is managed through agile
methods and ticket request system either on GitHub (for MOOSE) or on the defined software repository
for this application.
!template-end!

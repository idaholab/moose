!template load file=sqa/module_sdd.md.template category=ray_tracing module=Ray Tracing

!template! item key=introduction
The [!ac](MOOSE) {{module}} module is based on the MOOSE framework and thus inherits
the unique features and base characteristics of the framework, as outlined in the [framework_sdd.md].
Specific details unique to the module are outlined in this document.
!template-end!

!template! item key=system-scope
!include ray_tracing_srs.md start=system-scope-begin end=system-scope-finish
!template-end!

!template! item key=dependencies-and-limitations
The {{module}} module inherits the
[software dependencies and limitations of the MOOSE framework](framework_sdd.md#dependencies-and-limitations).
The {{module}} module is in its relative infancy, so it may not have all the features desired by potential users.
Notable features in development are the ability to trace in unmeshed cavities and from sideset-to-sideset ignoring
the volumetric mesh intersections.
!template-end!

!template! item key=design-stakeholders
!include framework_sdd.md start=design-stakeholders-begin end=design-stakeholders-finish
!template-end!

!template! item key=system-design
The {{module}} module inherits the wide range of pluggable systems from MOOSE. More
information regarding MOOSE system design can be found in the framework [framework_sdd.md#system-design]
section.

The module is designed in the same pluggable manner to be widely interoperable with other modules and MOOSE-based applications.
Specifically, it is centered around [ray tracing study](userobjects/RayTracingStudy.md) objects that are in charge of creating, initializing, propagating and finalizing
rays. These rays can carry data as they propagate through the mesh. The interaction of rays and this attached data with
the local elements are defined using [RayKernels](syntax/RayKernels/index.md). Similar interaction with sidesets are defined using
[RayBCs](syntax/RayBCs/index.md). 

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

!syntax complete subsystems=False actions=False objects=False groups=RayTracingApp

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

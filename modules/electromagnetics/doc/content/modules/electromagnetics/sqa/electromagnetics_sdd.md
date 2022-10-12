!template load file=sqa/module_sdd.md.template module=Electromagnetics category=electromagnetics

!template! item key=introduction
The [!ac](MOOSE) Electromagnetics module is based on the [!ac](MOOSE) framework, thus it inherits
the unique features and base characteristics of the framework, as outlined in the [framework_sdd.md].
Specific details unique to the module are outlined in this document.
!template-end!

!template! item key=system-scope
The purpose of this software is to augment the core [!ac](MOOSE) framework to provide libraries and
capability related to solving Maxwell's equations and other concepts of electrodynamics using the
[!ac](FEM).  Scope items specific to the framework that are therefore components of the Electromagnetics
module are given in the framework [framework_sdd.md#system-scope]. A brief overview of the main components
of the Electromagnetics module that exist beyond those of the framework are listed here:

| Component | Description |
| :- | :- |
| Electromagnetics module library | The optional physics library that may be used in an application to provide electrodynamics capability |
| benchmarks | A set of complete examples (within the module "test/tests" directory) demonstrating the use of the Electromagnetics module as well as verifying physical capabilities |
| unit | An application for unit testing individual Electromagnetics module classes or methods of C++ code specific to the module |
!template-end!

!template! item key=dependencies-and-limitations
The Electromagnetics module, having its base on the [!ac](MOOSE) framework, inherits MOOSE's software
dependencies. The scope of the module is evolving constantly based on funding, resources, priorities,
and laboratory direction. However, like [!ac](MOOSE), features and bugs can be offloaded to developers
with appropriate levels of domain knowledge and direction from the module design team. The primary list
of software dependencies can be found in the framework [framework_sdd.md#dependencies-and-limitations],
as it is identical to that of [!ac](MOOSE). There are currently no additional required software dependencies
for using electromagnetics module code and models.
!template-end!

!template! item key=design-stakeholders
!include framework_sdd.md start=design-stakeholders-begin end=design-stakeholders-finish
!template-end!

!template! item key=system-design
The Electromagnetics module inherits the wide range of pluggable systems from [!ac](MOOSE). More
information regarding [!ac](MOOSE) system design can be found in the framework [framework_sdd.md#system-design]
section. As a physics module/library, the electromagnetics module contains several physical models
related to electrodynamics. These are summarized via documentation of the demonstration benchmarks
and verification examples on the [module home page](electromagnetics/index.md). Documentation for
each object, data structure, and process specific to the module are kept up-to-date alongside the
[!ac](MOOSE) documentation. Expected failure modes and error conditions are accounted for via
regression testing, and error conditions are noted in object documentation where applicable.
!template-end!

!template! item key=system-structure
The architecture of the Electromagnetics module consists of a core and several pluggable systems (both
inherited from the MOOSE framework). The core of MOOSE consists of a number of key objects responsible
for setting up and managing the user-defined objects of a finite element simulation. This core set of
objects has limited extendability and exist for every simulation configuration that the module is
capable of running.

!syntax complete subsystems=False actions=False objects=False groups=ElectromagneticsApp

!style halign=left
The MooseApp is the top-level object used to hold all of the other objects in a simulation. In a
normal simulation a single MooseApp object is created and "run()". This object uses its Factory
objects to build user defined objects which are stored in a series of Warehouse objects and
executed. The Finite Element data is stored in the Systems and Assembly object while the domain
information (the Mesh) is stored in the Mesh object. A series of threaded loops are used to run
parallel calculations on the objects created and stored within the warehouses.

MOOSE's pluggable systems are documented on [MOOSE website](https://mooseframework.inl.gov), and those
for the Electromagnetics module are on this webpage, accessible through the high-level system links above.
Each of these systems has a set of defined polymorphic interfaces and are designed to accomplish a
specific task within the simulation. The design of these systems is fluid and is managed through agile
methods and ticket request system either on GitHub (for MOOSE) or on the defined software repository
for this application.
!template-end!

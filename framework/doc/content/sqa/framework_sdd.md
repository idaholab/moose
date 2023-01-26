!template load file=sdd.md.template category=framework project=Framework

!template! item key=introduction
!! introduction-begin
Frameworks are a software development construct aiming to simplify the creation of specific classes
of applications through abstraction of low-level details. The main objective of creating a framework is
to provide an interface to application developers that saves time and provides advanced capabilities
not attainable otherwise. The [!ac](MOOSE) mission is just that: provide a framework for engineers and
scientists to build state-of-the-art, computationally scalable finite element or finite volume based
simulation tools.

[!ac](MOOSE) was conceived with one major objective: to be as easy and straightforward to use by
scientists and engineers as possible. [!ac](MOOSE) is meant to be approachable by non-computational
scientists who have systems of [!ac](PDEs) they need to solve. Every single
aspect of [!ac](MOOSE) was driven by this singular principle from the build system to the API to
the software development cycle.  At every turn, decisions were made to enable this class of users to
be successful with the framework.  The pursuit of this goal has led to many of the unique features of
[!ac](MOOSE):

- A streamlined build system
- An API aimed at extensibility
- Straightforward APIs providing sensible default information
- Integrated, automatic, and rigorous testing
- Rapid, continuous integration development cycle
- Codified, rigorous path for contributing
- Applications are modular and composable

Each of these characteristics is meant to build trust in the framework by those attempting to use
it. For instance, the build system is the first thing potential framework users come into contact
with when they download a new software framework. Onerous dependency issues, complicated, hard to
follow instructions or build failure can all result in a user passing on the platform. Ultimately,
the decision to utilize a framework comes down to whether or not you trust the code in the framework
and those developing it to be able to support your desired use-case.  No matter the technical
capabilities of a framework, without trust users will look elsewhere. This is especially true of
those not trained in software development or computational science.

Developing trust in a framework goes beyond utilizing "best practices" for the code developed, it is
equally important that the framework itself is built upon tools that are trusted. For this reason,
[!ac](MOOSE) relies on a well-established code base of libMesh and PETSc.  The libMesh library
provides foundational capability for the finite element method and provides interfaces to
leading-edge numerical solution packages such as PETSc.

With these principles in mind, an open source, massively parallel, finite element, multiphysics
framework has been conceived.  [!ac](MOOSE) is an on-going project started in 2008 aimed toward a
common platform for creation of new multiphysics tools.  This document provides design details
pertinent to application developers as well as framework developers.

## Use Cases

The MOOSE Framework is targeted at two main groups of actors: Developers and Users. Developers are
the main use case. These are typically students and professionals trained in science and engineering
fields with some level of experience with coding but typically very little formal software
development training. The other user group is Users. Those who intend to use an application built
upon the framework without writing any computer code themselves. Instead they may modify or create
input files for driving a simulation, run the application, and analyze the results. All interactions
through MOOSE are primarily through the command-line interface and through a customizable block-based
input file.

!! introduction-finish
!template-end!

!template! item key=system-purpose
!! system-purpose-begin
The Software Design Description provided here is description of each object in the system. The pluggable
architecture of the framework makes [!ac](MOOSE) and [!ac](MOOSE)-based applications straightforward
to develop as each piece of end-user (developer) code that goes into the system follows a well-defined
interface for the underlying systems that those object plug into. These descriptions are provided
through developer-supplied "markdown" files that are required for all new objects that are developed
as part of the framework, modules and derivative applications. More information about the design
documentation can be found in [framework/documenting.md].
!! system-purpose-finish
!template-end!

!template! item key=system-scope
!! system-scope-begin
The purpose of this software is to provide several libraries that can be used to build an application
based upon the framework. Additionally, several utilities are provided
for assisting developers and users in end-to-end [!ac](FEM) analysis. A brief overview of the major
components are listed here:

| Component | Description |
| :- | :- |
| framework library | The base system from which all MOOSE-based applications are created |
| module libraries | Optional "physics" libraries that may be used in an application to provide capability |
| build system | The system responsible for creating applications for a series of libraries and applications |
| test harness | The extendable testing system for finding, scheduling, running, and reporting regression tests |
| "peacock" | The [!ac](GUI) for building input files, executing applications, and displaying results |
| MooseDocs | The extendable markdown system for MOOSE providing common documentation and requirements enforcement |
| "stork" | The script and templates for generating a new MOOSE-based application ready for building and testing |
| examples | A set of complete applications demonstrating the use of [!ac](MOOSE)'s pluggable systems |
| tutorials | Step by step guides to building up an application using [!ac](MOOSE)'s pluggable systems |
| unit | An application for unit testing individual classes or methods of C++ code |

!! system-scope-finish
!template-end!

!template! item key=dependencies-and-limitations
!! dependencies-and-limitations-begin
The [!ac](MOOSE) platform has several dependencies on other software packages and has scope that
is constantly evolving based upon funding, resources, priorities, and lab direction. However, the
software is open-source and many features and even bugs can be offloaded to developers with appropriate
levels of knowledge and direction from the main design team. The primary list of software dependencies
is listed below. This list is not meant to be exhaustive. Individual operating systems may require
specific packages to be installed prior to using MOOSE, which can be found on the
[Install MOOSE](getting_started/installation/index.md optional=True) pages.

| Software Dependency | Description |
| :- | :- |
| [libMesh](https://libmesh.github.io) | Finite Element Library and I/O routines |
| [PETSc](https://www.mcs.anl.gov/petsc/) | Solver Package |
| [hypre](https://computing.llnl.gov/projects/hypre-scalable-linear-solvers-multigrid-methods) | Multigrid Preconditioner |
| MPI | A distributed parallel processing library ([MPICH](https://www.mpich.org)) |

!media media/sqa/code_platform.png
       id=code_platform
       caption=A diagram of the MOOSE code platform.
       style=width=50%;

!! dependencies-and-limitations-finish
!template-end!

!template! item key=definitions
!! definitions-begin

- +Pull (Merge) Request+: A proposed change to the software (e.g. usually a code change, but may also include documentation, requirements, design, and/or testing).
- +Baseline+: A specification or product (e.g., project plan, maintenance and operations (M&O) plan, requirements, or design) that has been formally reviewed and agreed upon, that thereafter serves as the basis for use and further development, and that can be changed only by using an approved change control process [!citep](ASME-NQA-1-2008).
- +Validation+: Confirmation, through the provision of objective evidence (e.g., acceptance test), that the requirements for a specific intended use or application have been fulfilled [!citep](ISO-systems-software).
- +Verification+: (1) The process of: evaluating a system or component to determine whether the products of a given development phase satisfy the conditions imposed at the start of that phase. (2) Formal proof of program correctness (e.g., requirements, design, implementation reviews, system tests) [!citep](ISO-systems-software).

!! definitions-finish
!template-end!

!template! item key=acronyms
!acronym list
!template-end!

!template! item key=design-stakeholders
!! design-stakeholders-begin
Stakeholders for [!ac](MOOSE) include several of the funding sources including [!ac](DOE-NE)
and the [!ac](INL). However, Since [!ac](MOOSE) is an open-source project, several universities,
companies, and foreign governments have an interest in the development and maintenance of the
[!ac](MOOSE) project.
!! design-stakeholders-finish
!template-end!

!template! item key=stakeholder-design-concerns
!! stakeholder-design-concerns-begin
Concerns from many of the stakeholders are similar. These concerns include correctness, stability,
and performance. The mitigation plan for each of these can be addressed. For correctness, [!ac](MOOSE)
development requires either regression or unit testing for all new code added to the repository.
The project contains several comparisons against analytical solutions where possible and also
other verification methods such as [MMS](python/mms.md optional=True). For stability, [!ac](MOOSE) maintains
multiple branches to incorporate several layers of testing both internally and for dependent
applications. Finally, performance tests are also performed as part of the the normal testing suite
to monitor code change impacts to performance.
!! stakeholder-design-concerns-finish
!template-end!

!template! item key=system-design
!! system-design-begin
The MOOSE framework itself is composed of a wide range of pluggable systems. Each system is generally
composed of a single or small set of C++ objects intended to be specialized by a Developer to solve a
specific problem. To accomplish this design goal, MOOSE uses several modern object-oriented design
patterns. The primary overarching pattern is the "Factory Pattern". Users needing to extend MOOSE
may inherit from one of MOOSE's systems to providing an implementation meeting their needs. The
design of each of these systems is documented on the [MOOSE homepage](https://mooseframework.inl.gov).
Additionally, up-to-date documentation extracted from the source is maintained on the same documentation
site after every successful merge to MOOSE's stable branch. After these objects are created, they can be
registered with the framework and used immediately in a MOOSE input file.
!! system-design-finish
!template-end!

!template! item key=system-structure
!! system-structure-begin
The MOOSE framework architecture consists of a core and several pluggable systems. The core of MOOSE
consists of a number of key objects responsible for setting up and managing the user-defined objects
of a finite element simulation. This core set of objects has limited extendability and exist for
every simulation configuration that the framework is capable of running.

!syntax complete subsystems=False actions=False objects=False

The MooseApp is the top-level object used to hold all of the other objects in a simulation. In a
normal simulation a single MooseApp object is created and "run()". This object uses its Factory
objects to build user defined objects which are stored in a series of Warehouse objects and
executed. The Finite Element data is stored in the Systems and Assembly object while the domain
information (the Mesh) is stored in the Mesh object. A series of threaded loops are used to run
parallel calculations on the objects created and stored within the warehouses.

MOOSE's pluggable systems are documented on https://mooseframework.inl.gov. Each of these systems
has a set of defined polymorphic interfaces and are designed to accomplish a specific task within the
simulation. The design of these systems is fluid and is managed through agile methods and ticket
request system on the [MOOSE repository website](https://github.com/idaholab/moose).
!! system-structure-finish
!template-end!

!template! item key=data-design-and-control
!! data-design-and-control-begin
At a high level, the system is designed to process [!ac](HIT) input files to construct several
objects that will constitute an [!ac](FE) simulation. Some of the objects in the simulation may
in turn load other file-based resources to complete the simulation. Examples include meshes
or data files. The system will then assemble systems of equations and solve them using the
libraries of the [Code Platform](#dependencies-and-limitations). The system can then output the
solution in one or more supported output formats commonly used for visualization.
!! data-design-and-control-finish
!template-end!

!template! item key=human-machine-interface-design
!! human-machine-interface-design-begin
MOOSE is a command-line driven program. All interaction with MOOSE and MOOSE-based codes is
ultimately done through the command line. This is typical for [!ac](HPC) applications that use
the [!ac](MPI) interface for running on super computing clusters. Optional GUIs may be used
to assist in creating input files and launching executables on the command line.
!! human-machine-interface-design-finish
!template-end!

!template! item key=system-design-interface
!! system-design-interface-begin
All external system interaction is performed either through file [!ac](I/O) or through local
[!ac](API) calls. Neither the framework, nor the modules are designed to interact
with any external system directly through remote procedure calls. Any code to code coupling
performed using the framework are done directly through API calls either in
a static binary or after loading shared libraries.
!! system-design-interface-finish
!template-end!

!template! item key=security-structure
!! security-structure-begin
The framework does not require any elevated privileges to operate and does not
run any stateful services, daemons or other network programs. Distributed runs rely on the
[!ac](MPI) library.
!! security-structure-finish
!template-end!

!template! item key=requirements-cross-reference

!sqa cross-reference category=framework

!template-end!

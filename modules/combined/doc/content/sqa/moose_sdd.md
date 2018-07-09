!sqa load template=system_design_description.md.template
project=MOOSE

!sqa! item key=introduction
Frameworks are a software development construct aiming to simplify the creation of specific classes
of applications through abstraction of low-level details. The main object of creating a framework is
to provide an interface to application developers that saves time and provides advanced capabilities
not attainable otherwise. The !!acro MOOSE!!, mission is
just that: provide a framework for engineers and scientists to build state-of-the-art,
computationally scalable finite element based simulation tools.

!!acro MOOSE!! was conceived with one major objective: to be as easy and straightforward to use by
scientists and engineers as possible. !!acro MOOSE!! is meant to be approachable by non-computational
scientists who have systems of !!acro PDEs!! they need to solve. Every single
aspect of !!acro MOOSE!! was driven by this singular principle from the build system to the API to
the software development cycle.  At every turn, decisions were made to enable this class of users to
be successful with the framework.  The pursuit of this goal has led to many of the unique features of
!!acro MOOSE!!:

- A streamlined build system
- An API aimed at extensible
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
!!acro MOOSE!! relies on a well-established code base of libMesh and PETSc.  The libMesh library
provides foundational capability for the finite element method and provides interfaces to
leading-edge numerical solution packages such as PETSc.

With these principles in mind, an open source, massively parallel, finite element, multiphysics
framework has been conceived.  !!acro MOOSE!! is an on-going project started in 2008 aimed toward a
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

!sqa-end!


!sqa! item key=system-design
The MOOSE framework itself is composed of a wide range of pluggable systems. Each system is generally
composed of a single or small set of C++ objects intended to be specialized by a Developer to solve a
specific problem. To accomplish this design goal, MOOSE uses several modern object-oriented design
patterns. The primary overarching pattern is the "Factory Pattern". Users needing to extend MOOSE
may inherit from one of MOOSE's systems to providing an implementation meeting his or her needs. The
design of each of these systems is documented on the mooseframework.org wiki in the Tutorial
section. Additionally, up-to-date documentation extracted from the source is maintained on the the
mooseframework.org documentation site after every successful merge to MOOSE's stable branch. After
these objects are created, the can be registered with the framework and used immediately in a MOOSE
input file.
!sqa-end!

!sqa! item key=system-structure
The MOOSE framework architecture consists of a core and several pluggable systems. The core of MOOSE
consists of a number of key objects responsible for setting up and managing the user-defined objects
of a finite element simulation. This core set of objects has limited extendability and exist for
every simulation configuration that the framework is capable of running.

!alert construction
This list is automatically generated by MooseDocs, each system (as determined by the syntax dump from
the executable) is known and has a markdown page. It is assumed that each page contains the system
design. In the future these pages may require a "design" section, which this list could reference
directly.

!syntax list subsystems=True actions=False objects=False

The MooseApp is the top-level object used to hold all of the other objects in a simulation. In a
normal simulation a single MooseApp object is created and "run()". This object uses it's Factory
objects to build user defined objects which are stored in a series of Warehouse objects and
executed. The Finite Element data is stored in the Systems and Assembly object while the domain
information (the Mesh) is stored in the Mesh object. A series of threaded loops are used to run
parallel calculations on the objects created and stored within the warehouses.

MOOSE's pluggable systems are documented on the mooseframework.org wiki. Each of these systems has
set of defined polymorphic interfaces and are designed to accomplish a specific task within the
simulation. The design of these systems is fluid and is managed through agile methods and ticket
request system on the Github.org website.

!sqa-end!

!sqa! item key=requirements-cross-reference

!sqa cross-reference

!sqa-end!

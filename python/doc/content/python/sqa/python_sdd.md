!template load file=sqa/sdd.md.template project=MOOSE Tools

!template! item key=introduction
The main objective of creating a set of support utilities such as [!ac](MOOSE) Tools is to provide a
resource to application and framework developers that saves time and provides useful capabilities
not attainable otherwise in a tailored way. The [!ac](MOOSE) Tools mission is just that: provide a
set of capability for engineers and scientists using the [!ac](MOOSE) framework to support testing,
verification, analysis, and documentation of their simulations and applications.

[!ac](MOOSE) Tools was designed to be as easy and straightforward to use by scientists and engineers
as possible. [!ac](MOOSE) is meant to be approachable by non-computational scientists who use [!ac](MOOSE)
to perform their research. Thus, [!ac](MOOSE) Tools has grown and developed alongside [!ac](MOOSE) to
be as effective as possible in supporting these aims. This has led to many of the unique features of
[!ac](MOOSE) Tools:

- A general test platform for arbitrarily located test cases within a code base and facilitating
  highly-parallel testing
- A general documentation platform that can use source code, markdown, and code input to create flexible
  websites, reports, and presentations directly from the code repository
- Utilities for manufactured solution code verification
- Utilities for data manipulation, conversion, and differencing
- Integrated, automatic, and rigorous testing
- Rapid, continuous integration development cycle
- Codified, rigorous path for contributing

Each of these characteristics is meant to build trust in the framework by those attempting to use
it by providing comprehensive solutions for their general needs, coupled with an extensible system
for new requirements. Ultimately, the decision to utilize code contained in [!ac](MOOSE) Tools comes
down to whether or not you trust the code in the utilities and those developing it to be able to support
your desired use-case.  No matter the technical capabilities of a code, without trust users will look
elsewhere. This is especially true of those not trained in software development or computational science.

Developing trust in a code base goes beyond utilizing "best practices" for the code developed, it is
equally important that the code itself is built upon tools that are trusted. For this reason,
[!ac](MOOSE) Tools relies on well-known community-driven libraries and utilities to perform some of
its functions. See the [python_sll.md] for more information.

With these principles in mind, an open source, flexible tool to complement the greater [!ac](MOOSE)
ecosystem has been conceived.  [!ac](MOOSE) Tools is an on-going project started to support [!ac](MOOSE)
itself and is aimed toward a common platform for code testing, documentation, verification, and data
support. This document provides design details pertinent to application developers as well as framework
developers.

## Use Cases

The set of [!ac](MOOSE) Tools utilities are targeted at two main groups of actors: Developers and
Users. Developers are the main use case. These are typically students and professionals trained in
science and engineering fields with some level of experience with coding but typically very little
formal software development training. The other user group is Users. Those who intend to use an
application built upon the [!ac](MOOSE) framework without writing any computer code themselves.
Instead they may modify or create input files for driving a simulation, run the application, and
analyze the results. This analysis, testing and verification of newly created code, and documentation
support are then handled through [!ac](MOOSE) Tools utilities. All interactions using [!ac](MOOSE)
Tools are primarily through the command-line interface.
!template-end!

!template! item key=system-purpose
!include python/sqa/python_system_purpose.md
!template-end!

!template! item key=system-scope
!include python/sqa/python_system_scope.md
!template-end!

!template! item key=dependencies-and-limitations
[!ac](MOOSE) Tools has several dependencies on other software packages and has scope that
is constantly evolving based upon funding, resources, priorities, and lab direction as the [!ac](MOOSE)
framework expands. However, the software is open-source and many features and even bugs can be offloaded
to developers with appropriate levels of knowledge and direction from the main design team. The primary
list of software dependencies is listed in the [python/sqa/python_sll.md]. This list is not meant to
be exhaustive. Individual operating systems may require specific packages to be installed prior to
using [!ac](MOOSE) Tools, which can be found on the [Install MOOSE](getting_started/installation/index.md optional=True) pages.
!template-end!

!template item key=definitions
!include framework_sdd.md start=definitions-begin end=definitions-finish

!template! item key=acronyms
!acronym list
!template-end!

!template item key=design-stakeholders
!include framework_sdd.md start=design-stakeholders-begin end=design-stakeholders-finish replace=['[!ac](MOOSE)', '[!ac](MOOSE) Tools']

!template item key=stakeholder-design-concerns
!include framework_sdd.md start=stakeholder-design-concerns-begin end=stakeholder-design-concerns-finish

!template! item key=system-design
[!ac](MOOSE) Tools is composed of a wide range of utilities. Each utility is generally
composed of a single or small set of Python objects or interfaces intended to be specialized and
combined by a Developer to support a specific code base, application, or simulation. To accomplish
this design goal, [!ac](MOOSE) Tools utilities generally use a particular design pattern, consisting
of a central core of common functionality extended by code designed for specific tasks. Users needing
to extend or create new [!ac](MOOSE) Tools utilities may, in many cases, use the main code as Python
packages and specialize or modify it to provide an implementation meeting their needs. The
design of each of these systems is documented on the [MOOSE homepage](https://mooseframework.inl.gov).
Additionally, up-to-date documentation extracted from the source is maintained on the same documentation
site after every successful merge to MOOSE's stable branch.
!template-end!

!template! item key=system-structure
The [!ac](MOOSE) Tools system consists of multiple utilities. Each utility has its own unique structure,
generally containing a set of core functionality surrounded by extensions or flexible methods for
user/developer usage. Links to design documentation for [!ac](MOOSE) Tools can be found below in [python-packages].

!include python/index.md start=!table id=python-packages end=## Setup

The design of these utilities is fluid and is managed through agile methods and ticket request system
on the [MOOSE repository website](https://github.com/idaholab/moose).
!template-end!

!template! item key=data-design-and-control
At a high level, the system is designed to process configuration input, source code documentation, and
other support files in order to facilitate its design (supporting the [!ac](MOOSE) framework and
[!ac](MOOSE)-based applications). Some components of the utilities may in turn load other file-based
resources to complete its processes. Examples include secondary configuration files or data files.
The system will then assemble its pre-requisites and perform its function using the libraries
of the [Code Platform](#dependencies-and-limitations). The system can then output various outputs
associated with its design function -- a website, test results, manufactured solution, translated
data, etc. An example of this is the MooseDocs rendering of documentation webpages -- configuration
files, markdown documentation, code documentation, input files, and source code itself will be assembled
and then rendered to a functional conclusion (website, presentation, or document) using the MooseDocs
code extension systems.
!template-end!

!template! item key=human-machine-interface-design
[!ac](MOOSE) Tools utilities are command-line driven programs. All interaction with MOOSE and MOOSE-based
codes and data is ultimately done through the command line. This is typical for [!ac](HPC) applications
that use the [!ac](MPI) interface for running on computing clusters.
!template-end!

!template! item key=system-design-interface
All external system interaction is performed either through file [!ac](I/O) or through local [!ac](API)
calls. [!ac](MOOSE) Tools is not designed to interact with any external system directly through remote
procedure calls.
!template-end!

!template item key=security-structure
!include framework_sdd.md start=security-structure-begin end=security-structure-finish replace=['The framework', '[!ac](MOOSE) Tools']

!template! item key=requirements-cross-reference

!sqa cross-reference category=python

!template-end!

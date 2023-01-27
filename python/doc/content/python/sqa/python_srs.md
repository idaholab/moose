!template load file=sqa/srs.md.template project=MOOSE Tools

!template! item key=system-purpose
!include python/sqa/python_system_purpose.md
!template-end!

!template! item key=system-scope
!include python/sqa/python_system_scope.md
!template-end!

!template! item key=system-context
[!ac](MOOSE) Tools utilities are command-line driven applications. This is typical for high-performance
software that is designed to run across several nodes of a cluster system. As such, all of the usage
of the software is through any standard terminal program generally available on all supported
operating systems. Similarly, for the purpose of interacting through the software, there is only
a single user, "the user", which interacts with the software through the command-line. [!ac](MOOSE)
Tools does not maintain any back-end database or interact with any system daemons. It is an executable,
which may be launched from the command line and writes out various result files as it runs.

!media media/sqa/usage_diagram_uml.svg
       id=usage_diagram
       caption=Usage of [!ac](MOOSE) and [!ac](MOOSE)-based applications. [!ac](MOOSE) Tools usage
               is similar.
       style=width:50%;
!template-end!

!template! item key=system-functions
Since [!ac](MOOSE) Tools is a command-line driven application, all functionality provided in the provided
set of utilities is operated through the use of standard UNIX command line flags, YAML configuration
files, Python-based scripts, and user-created [MooseDocs/specification.md]. Many of the [!ac](MOOSE)
Tools utilities are completely extendable so individual design pages should be consulted for specific
behaviors of each system and extension.
!template-end!

!template! item key=user-characteristics
!! user-characteristics-begin

- +[!ac](MOOSE) Tools Developers+: These are the core developers of the project. They will be responsible
  for following and enforcing the appropriate software development standards. They will be
  responsible for designing, implementing and maintaining the software.

- +Developers+: A Scientist or Engineer that utilizes [!ac](MOOSE) Tools to build their own
  tools, extensions, or scripts to assist in the development of a project - either [!ac](MOOSE) or
  MOOSE-based applications or custom code to which [!ac](MOOSE) Tools has been adapted. This user will
  typically have a background in modeling and simulation techniques and/or numerical analysis but may
  only have a limited skill-set when it comes to the Python language. This is our primary focus group.
  In many cases these developers will be encouraged to give their code back to the [!ac](MOOSE) Tools
  maintainers.

- +Analysts+: These are users that will run the code and perform various analysis and testing on the
  simulations they perform. These users may interact with developers of the system requesting new features
  and reporting bugs found and will typically make heavy use of the pre-built scripts, configuration
  files, and the [MooseDown](python/MooseDocs/specification.md) file format.

!template-end!

!template! item key=assumptions-and-dependencies
!include python/sqa/python_assumptions_and_dependencies.md
!template-end!

!template! item key=definitions
!include framework_srs.md start=definitions-begin end=definitions-finish
!template-end!

!template! item key=acronyms
!acronym list
!template-end!

!template item key=minimum-requirements
!include sqa/minimum_requirements.md start=### Minimum System Requirements include-start=False

!template item key=functional-requirements
!sqa requirements link=False collections=FUNCTIONAL category=python

!template item key=usability-requirements
!sqa requirements link=False collections=USABILITY category=python

!template item key=performance-requirements
!sqa requirements link=False collections=PERFORMANCE category=python

!template item key=system-interfaces-requirements
!sqa requirements link=False collections=SYSTEM category=python

!template item key=human-system-integration
[!ac](MOOSE) Tools contains command line driven applications which conform to all standard terminal
behaviors. Specific human system interaction accommodations shall be a function of the end-user's
terminal. Optional accommodations will be outlined in utility design documents.

!template item key=maintainability
!include framework_srs.md start=maintainability-begin end=maintainability-finish

!template item key=reliability
The regression test suite will cover a sufficient amount of [!ac](MOOSE) Tools code such that core
utility functionality is maintained at all times. Known regressions will be recorded and tracked
(see [#maintainability]) to an independent and satisfactory resolution.

!template! item key=system-modes
[!ac](MOOSE) Tools utilities are typically used within other scripts or codes to facilitate particular
functionality. Some, like the [python/MooseDocs/index.md] take command line arguments and run in normal
execution mode when a configuration file is supplied. However, due to the nature of [!ac](MOOSE) Tools
being a collection of utilities, each of them have their own modes and states of operation. Thus, the
best resource is their design and operation documentation, which can be shown below in [python-packages].

!include python/index.md start=!table id=python-packages end=## Setup
!template-end!

!template! item key=physical-characteristics
[!ac](MOOSE) Tools is software only with no associated physical media. See [#system-requirements] for
a description of the minimum required hardware necessary for running a [!ac](MOOSE) Tools (as well as
a [!ac](MOOSE)-based application).
!template-end!

!template item key=environmental-conditions
!include framework_srs.md start=environmental-conditions-begin end=environmental-conditions-finish

!template item key=system-security
[!ac](MOOSE) Tools utilities have no requirements or special needs related to system security. They
are designed to run completely in user-space with no elevated privileges required or recommended.

!template item key=information-management
!include framework_srs.md start=information-management-begin end=information-management-finish replace=['The core framework', '[!ac](MOOSE) Tools']

!template item key=policies-and-regulations
As [!ac](MOOSE) Tools is housed within and alongside [!ac](MOOSE) and [!ac](MOOSE)-based applications,
it must comply with all export control restrictions.

!template item key=system-life-cycle
!include framework_srs.md start=system-life-cycle-begin end=system-life-cycle-finish

!template item key=packaging
!include framework_srs.md start=packaging-begin end=packaging-finish

!template item key=verification
The regression test suite will employ several verification tests using comparison against known
solutions or code output.

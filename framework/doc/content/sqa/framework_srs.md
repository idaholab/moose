!template load file=srs.md.template project=Framework category=framework

!template! item key=system-purpose
!include sqa/system_purpose.md
!template-end!

!template! item key=system-scope
!include sqa/system_scope.md
!template-end!

!template! item key=system-context
!! system-context-begin
[!ac](MOOSE) is a command-line driven application. This is typical for high-performance software
that is designed to run across several nodes of a cluster system. As such, all of the usage
of the software is through any standard terminal program generally available on all supported
operating systems. Similarly, for the purpose of interacting through the software, there is only
a single user, "the user", which interacts with the software through the command-line. MOOSE does
not maintain any back-end database or interact with any system daemons. It is an executable,
which may be launched from the command line and writes out various result files as it runs.

!media media/sqa/usage_diagram_uml.svg
       id=usage_diagram
       caption=Usage of [!ac](MOOSE) and [!ac](MOOSE)-based applications.
       style=width:50%;
!! system-context-finish
!template-end!

!template! item key=system-functions
!! system-functions-begin
Since [!ac](MOOSE) is a command-line driven application, all functionality provided in the framework
is operated through the use of standard UNIX command line flags and the extendable MOOSE input file.
The framework is completely extendable so individual design pages should be consulted for specific
behaviors of each user-defined object.
!! system-functions-finish
!template-end!

!template! item key=user-characteristics
!! user-characteristics-begin

- +Framework Developers+: These are the core developers of the framework. They will be responsible
  for following and enforcing the appropriate software development standards. They will be
  responsible for designing, implementing and maintaining the software.

- +Developers+: A Scientist or Engineer that utilizes the framework to build their own
  application. This user will typically have a background in modeling and simulation techniques
  and/or numerical analysis but may only have a limited skill-set when it comes to object-oriented
  coding and the C++ language. This is our primary focus group. In many cases these developers will
  be encouraged to give their code back to the framework maintainers.

- +Analysts+: These are users that will run the code and perform various analysis on the simulations
  they perform.  These users may interact with developers of the system requesting new features and
  reporting bugs found and will typically make heavy use of the input file format.

!! user-characteristics-finish
!template-end!

!template! item key=assumptions-and-dependencies
!include sqa/assumptions_and_dependencies.md
!template-end!

!template! item key=definitions
!! definitions-begin

- +Verification+: (1) The process of: evaluating a system or component to determine whether the
  products of a given development phase satisfy the conditions imposed at the start of that
  phase. (2) Formal proof of program correctness (e.g., requirements, design, implementation
  reviews, system tests) [!citep](ISO-systems-software).

!! definitions-finish
!template-end!

!template! item key=acronyms
!acronym list
!template-end!

!template! item key=minimum-requirements
!! minimum-requirements-begin

!include sqa/minimum_requirements.md start=### Minimum System Requirements include-start=False

!! minimum-requirements-finish
!template-end!

!template item key=functional-requirements
!sqa requirements link=False collections=FUNCTIONAL category={{category}}

!template item key=usability-requirements
!sqa requirements link=False collections=USABILITY category={{category}}

!template item key=performance-requirements
!sqa requirements link=False collections=PERFORMANCE category={{category}}

!template item key=system-interfaces-requirements
!sqa requirements link=False collections=SYSTEM category={{category}}

!template! item key=human-system-integration
!! human-system-integration-begin
[!ac](MOOSE) is a command line driven application which conforms to all standard terminal
behaviors. Specific human system interaction accommodations shall be a function of the end-user's
terminal. MOOSE does support optional coloring within the terminal's ability to display color,
which may be disabled.
!! human-system-integration-finish
!template-end!


!template! item key=maintainability
!! maintainability-begin

- The latest working version (defined as the version that passes all tests in the current regression
  test suite) shall be publicly available at all times through the repository host provider.
- Flaws identified in the system shall be reported and tracked in a ticket or issue based system. The
  technical lead will determine the severity and priority of all reported issues and assign resources
  at their discretion to resolve identified issues.
- The software maintainers will entertain all proposed changes to the system in a timely manner
  (within two business days).
- The core framework in its entirety will be made publicly available under the [!ac](LGPL)
  version 2.0 license.

!! maintainability-finish
!template-end!


!template! item key=reliability
!! reliability-begin
The regression test suite will cover at least 80% of all lines of code at all times. Known
regressions will be recorded and tracked (see [#maintainability]) to an independent and
satisfactory resolution.
!! reliability-finish
!template-end!

!template! item key=information-management
!! information-management-begin
The core framework in its entirety will be made publicly available on an appropriate repository
hosting site. Day-to-day backups and security services will be provided by the hosting service. More
information about MOOSE backups of the public repository on [!ac](INL)-hosted services can be found
on the following page: [sqa/github_backup.md]
!! information-management-finish
!template-end!

!template! item key=verification
!! verification-begin
The regression test suite will employ several verification tests using comparison against known
analytical solutions, the method of manufactured solutions, and convergence rate analysis.
!! verification-finish
!template-end!

!template! item key=system-modes
!! system-modes-begin
[!ac](MOOSE) applications normally run in normal execution  mode when an input file is supplied. However,
there are a few other modes that can be triggered with various command line flags as indicated here:

| Command Line Flag | Description of mode |
| :- | :- |
| `-i <input_file>` | Normal execution mode |
| `--split-mesh <splits>` | Read the mesh block splitting the mesh into two or more pieces for use in a subsequent run |
| `--use-split` | (implies -i flag) Execute the the simulation but use pre-split mesh files instead of the mesh from the input file |
| `--yaml` | Output all object descriptions and available parameters in YAML format |
| `--json` | Output all object descriptions and available parameters in JSON format |
| `--syntax` | Output all registered syntax |
| `--registry` | Output all known objects and actions |
| `--registry-hit` | Output all known objects and actions in HIT format |
| `--mesh-only` (implies -i flag) | Run only the mesh related tasks and output the final mesh that would be used for the simulation |
| `--start-in-debugger <debugger>` | Start the simulation attached to the supplied debugger |

!alert note
The list of system-modes may not be extensive as the system is designed to be extendable to end-user applications.
The complete list of command line options for applications can be obtained by running the executable with
zero arguments. See the [command line usage](command_line_usage.md optional=True).
!! system-modes-finish
!template-end!

!template! item key=physical-characteristics
!! physical-characteristics-begin
[!ac](MOOSE) is software only with no associated physical media. See [#system-requirements] for a description
of the minimum required hardware necessary for running a [!ac](MOOSE)-based application.
!! physical-characteristics-finish
!template-end!

!template! item key=environmental-conditions
!! environmental-conditions-begin
Not Applicable
!! environmental-conditions-finish
!template-end!

!template! item key=system-security
!! system-security-begin
[!ac](MOOSE) based applications have no requirements or special needs related to system security. The framework
is designed to run completely in user-space with no elevated privileges required nor recommended.
!! system-security-finish
!template-end!

!template! item key=policies-and-regulations
!! policies-and-regulations-begin
[!ac](MOOSE)-based applications must comply with all export control restrictions.
!! policies-and-regulations-finish
!template-end!

!template! item key=system-life-cycle
!! system-life-cycle-begin
[!ac](MOOSE)-based development follows various agile methods. The system is continuously built and deployed in
a piecemeal fashion since objects within the system are more or less independent. Every new object requires a test,
which in turn requires an associated requirement and design description. Some [!ac](MOOSE)-based development
teams follow the [!ac](NQA-1) standards.
!! system-life-cycle-finish
!template-end!

!template! item key=packaging
!! packaging-begin
No special requirements are needed for packaging or shipping any media containing [!ac](MOOSE) source code. However,
some [!ac](MOOSE)-based applications may be export-controlled, in which case all export control restrictions must
be adhered to when packaging and shipping media.
!! packaging-finish
!template-end!

!template load file=srs.md.template category=framework project=Framework

!template! item key=system-purpose
The [!ac](MOOSE) is a tool for solving complex coupled
Multiphysics equations using the finite element method. [!ac](MOOSE) uses an object-oriented design
to abstract data structure management, parallelism, threading and compiling while providing an easy
to use interface targeted at engineers that may not have a lot of software development
experience. [!ac](MOOSE)  will require extreme scalability and flexibility when compared to other
FEM frameworks. For instance, [!ac](MOOSE) needs the ability to run extremely complex material
models, or even third-party applications within a parallel simulation without sacrificing
parallelism. This capability is in contrast to what is often seen in commercial packages, where
custom material models can limit the parallel scalability, forcing serial runs in the most severe
cases. When comparing high-end capabilities, many [!ac](MOOSE) competitors target modest-sized
clusters with just a few thousand processing cores. [!ac](MOOSE), however, will be required to
routinely executed on much larger clusters with scalability to clusters available in the top 500
systems ([top500.org](http://www.top500.org)). [!ac](MOOSE) will also be targeted at smaller systems
such as high-end laptop computers.

The design goal of [!ac](MOOSE) is to give developers ultimate control over their physical models
and applications. Designing new models or solving completely new classes of problems will be
accomplished by writing standard C++ source code within the framework's class hierarchy. Scientists
and engineers will be free to implement completely new algorithms using pieces of the framework where
possible, and extending the framework's capabilities where it makes sense to do so. Commercial
applications do not have this capability, and instead opt for either a more rigid parameter system or
a limited application-specific metalanguage.
!template-end!

!template! item key=system-scope
[!ac](MOOSE)'s scope is to provide a set of interfaces for building [!ac](FEM)
simulations. Abstractions to all underlying libraries are provided.

Solving coupled problems where competing physical phenomena impact one and other in a significant
nonlinear fashion represents a serious challenge to several solution strategies. Small perturbations
in strongly-coupled parameters often have very large adverse effects on convergence behavior. These
adverse effects are compounded as additional physics are added to a model. To overcome these
challenges, [!ac](MOOSE) employs three distinct yet compatible systems for solving these types of
problems.

First, an advanced numerical technique called the [!ac](JFNK) method is
employed to solve the most fully-coupled physics in an accurate, consistent way. An example of this
would be the effect of temperature on the expansion or contraction of a material. While the
[!ac](JFNK) numerical method is very effective at solving fully-coupled equations, it can also be
computationally expensive. Plus, not all physical phenomena in a given model are truly coupled to one
another. For instance, in a reactor, the speed of the coolant flow may not have any direct effect on
the complex chemical reactions taking place inside the fuel rods.  We call such models
"loosely-coupled". A robust, scalable system must strike the proper balance between the various
modeling strategies to avoid performing unnecessary computations or incorrectly predicting behavior
in situations such as these.

[!ac](MOOSE)'s Multiapp system will allow modelers to group physics into logical categories where
[!ac](MOOSE) can solve some groups fully-coupled and others loosely-coupled. The Multiapp system
goes even further by also supporting a "tightly-coupled" strategy, which falls somewhere between the
"fully-coupled" and "loosely-coupled" approaches. Several sets of physics can then be linked together
into logical hierarchies using any one of these coupling strategies, allowing for several potential
solution strategies. For instance, a complex nuclear reactor model might consist of several
tightly-coupled systems of fully-coupled equations.

Finally, [!ac](MOOSE)'s Transfers system ties all of the physics groups contained within the
Multiapp system together and allows for full control over the flow of information among the various
groups. This capability bridges physical phenomena from several different complementary scales
simultaneously. When these three [!ac](MOOSE) systems are combined, myriad coupling combinations
are possible. In all cases, the [!ac](MOOSE) framework handles the parallel communication, input,
output and execution of the underlying simulation. By handling these computer science tasks, the
[!ac](MOOSE) framework keeps modelers focused on doing research.

[!ac](MOOSE) innovates by building advanced simulation capabilities on top of the very best
available software technologies in a way that makes them widely accessible for innovative
research. [!ac](MOOSE) is equally capable of solving small models on common laptops and the very
biggest FEM models ever attempted---all without any major changes to configuration or source
code. Since its inception, the [!ac](MOOSE) project has focused on both developer and computational
efficiency. Improved developer efficiency is achieved by leveraging existing algorithms and
technologies from several leading open-source packages. Additionally, [!ac](MOOSE) uses several
complementary parallel technologies (both the distributed-memory message passing paradigm and
shared-memory thread-based approaches are used) to lay an efficient computational foundation for
development. Using existing open technologies in this manner helps the developers reduce the scope of
the project and keeps the size of the [!ac](MOOSE) code base maintainable. This approach provides
users with state-of-the-art finite element and solver technology as a basis for the advanced coupling
and solution strategies mentioned previously.

[!ac](MOOSE)'s developers work openly with other package developers to make sure that cutting-edge
technologies are available through [!ac](MOOSE), providing researchers with competitive research
opportunities. [!ac](MOOSE) maintains a set of objects that hide parallel interfaces while exposing
advanced spatial and temporal coupling algorithms in the framework.  This accessible approach places
developmental technology into the hands of scientists and engineers, which can speed the pace of
scientific discovery.
!template-end!

!template! item key=system-context
[!ac](MOOSE) is a command-line driven application. This is typical for a high-performance software
that is designed to run across several nodes of a cluster system. As such, all of the usage
of the software is through any standard terminal program generally available on all supported
operating systems. Similarly, for the purpose of interacting through the software, there is only
a single user, "the user", which interacts with the software through the command-line. MOOSE does
not maintain any back-end database or interact with any system daemons. It is a simple executable,
which may be launched from the command line and writes out various result files as it runs.

!media media/sqa/usage_diagram_uml.svg
       id=usage_diagram
       caption=Usage of [!ac](MOOSE) and [!ac](MOOSE)-based applications.
       style=width:50%;
!template-end!

!template! item key=system-functions
Since [!ac](MOOSE) is a command-line driven application, all functionality provided in the framework
is operated through the use of standard UNIX command line flags and the extendable MOOSE input file.
The framework is completely extendable so individual design pages should be consulted for specific
behaviors of each user-defined object.
!template-end!

!template! item key=user-characteristics

- +Framework Developers+: These are the core developers of the framework. They will be responsible
  for following and enforcing the appropriate software development standards. They will be
  responsible for designing, implementing and maintaining the software.

- +Developers+: A Scientist or Engineer that utilizes the framework to build his or her own
  application. This user will typically have a background in modeling and simulation techniques
  and/or numerical analysis but may only have a limited skill-set when it comes to object-oriented
  coding and the C++ language. This is our primary focus group.  In many cases these developers will
  be encouraged to give their code back to the framework maintainers.

- +Analysts+: These are users that will run the code and perform various analysis on the simulations
  they perform.  These users may interact with developers of the system requesting new features and
  reporting bugs found and will typically make heavy use of the input file format.

!template-end!


!template! item key=assumptions-and-dependencies
The software should be designed with the fewest possible constraints. Ideally the software should run
on a wide variety of evolving hardware so it should follow well-adopted standards and guidelines. The
software should run on any [!ac](POSIX) compliant system. The software will also make use FEM and
numerical libraries that run on [!ac](POSIX) systems as well. The main interface for the software
will be command line based with no assumptions requiring advanced terminal capabilities such as
coloring and line control.
!template-end!

!template! item key=definitions
- +Verification+: (1) The process of: evaluating a system or component to determine whether the
  products of a given development phase satisfy the conditions imposed at the start of that
  phase. (2) Formal proof of program correctness (e.g., requirements, design, implementation reviews,
  system tests) [!citep](ISO-systems-software).
!template-end!

!template! item key=acronyms
!acronym list
!template-end!


!template! item key=system-requirements
The creation of the requirements for [!ac](MOOSE) is an ongoing progress as new objects are added.
The [#functional-requirements] are generated from test specifications, which are required with
each addition to the MOOSE framework or its modules.
!template-end!

!template! item key=minimum-requirements
- A [!ac](POSIX) compliant Unix including the two most recent versions of MacOS and most current
  versions of Linux.
- 4 GB of RAM for optimized compilation (8 GB for debug compilation), 2 GB per core execution
- 100 GB disk space
- C++11 compatible compiler (GCC, Clang)
- Python 2.6+
- Git
!template-end!

!template! item key=functional-requirements
!sqa requirements link=False category={{category}}
!template-end!

!template! item key=useability-requirements
!sqa requirements-matrix prefix=U
- The system will be command-line and input file driven.
- The system shall return usage messages when unidentified arguments or incorrectly used arguments are passed.
- The system shall provide diagnostics when the input file fails to parse, or the format is incorrect.
- The system will provide on screen information about the simulation and performance characteristics of the solves under normal operating conditions.
!template-end!

!template! item key=performance-requirements
!sqa requirements-matrix prefix=P
- The system will support multi-process distributed memory execution.
- The system will support multi-process shared memory execution.
- The system will support execution on Unix-based laptops.
- The system will support execution on Unix-based workstation systems.
- The system will support execution on large Unix-based cluster systems.
!template-end!

!template! item key=system-interfaces
!sqa requirements-matrix prefix=S
- The system shall support POSIX compliant systems.
- The system shall support the Message Passing Interface (MPI) standard.
- The system shall support POSIX ``pthreads''.
- The system shall support Intel Threaded Building Blocks (TBB) interface.
- The system shall support the OpenMP threading interface.
!template-end!

!template! item key=human-system-integration
[!ac](MOOSE) is a command line driven application which conforms to all standard terminal
behaviors. Specific human system interaction accommodations shall be a function of the end-user's
terminal. MOOSE does support optional coloring within the terminal's ability to display color,
which may be disabled.
!template-end!


!template! item key=maintainability
- The latest working version (defined as the version that passes all tests in the current regression
  test suite) shall be publicly available at all times through the repository host provider.
- Flaws identified in the system shall be reported and tracked in a ticket or issue based system. The
  technical lead will determine the severity and priority of all reported issues and assign resources
  at his or her discretion to resolve identified issues.
- The software maintainers will entertain all proposed changes to the system in a timely manner
  (within two business days).
- The core framework in its entirety will be made publicly available under the [!ac](LGPL)
  version 2.0 license.
!template-end!


!template item key=reliability
The regression test suite will cover at least 80% of all lines of code at all times. Known
regressions will be recorded and tracked (see [#maintainability]) to an independent and
satisfactory resolution.
!template-end!

!template item key=information-management
The core framework in its entirety will be made publicly available on an appropriate repository
hosting site. Backups and security services will be provided by the hosting service.
!template-end!

!template item key=verification
The regression test suite will employ several verification tests using comparison against known
analytical solutions, the method of manufactured solutions, and convergence rate analysis.
!template-end!

!template! item key=system-modes
[!ac](MOOSE) applications normally run in normal execution  mode when an input file is supplied. However
there are a few other modes that can be triggered with various command line flags as indicated here:

| Command Line Flag | Description of mode |
| :- | :- |
| `-i <input_file>` | Normal execution mode |
| `--split-mesh <splits>` | Read the mesh block splitting the mesh into two or more pieces for use in a subsequent run |
| `--use-split` | (inplies -i flag) Execute the the simulation but use pre-split mesh files instead of the mesh from the input file |
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
!template-end!

!template! item key=physical-characteristics
[!ac](MOOSE) is software only with no associated physical media. See [#system-requirements] for a description
of the minimum required hardware necessary for running a [!ac](MOOSE)-based application.
!template-end!

!template! item key=environmental-conditions
Not Applicable
!template-end!

!template! item key=system-security
[!ac](MOOSE) based applications have no requirements or special needs related to system-security. The framework
is designed to run completely in user-space with no elevated privileges required nor recommended.
!template-end!

!template! item key=policies-and-regulations
[!ac](MOOSE)-based applications must comply with all export control restrictions.
!template-end!

!template! item key=system-life-cycle
[!ac](MOOSE)-based development follows various agile methods. The system is continuously built and deployed in
a piecemeal fashion since objects within the system are more or less independent. Every new object requires a test,
which in turn requires an associated requirement and design description. Some [!ac](MOOSE)-based development
teams follow the [!ac](NQA-1) standards.
!template-end!

!template! item key=packaging
No special requirements are needed for packaging or shipping any media containing [!ac](MOOSE) source code. However,
some [!ac](MOOSE)-based applications maybe be export controlled in which case all export control restrictions must
be adhered to when packaging and shipping media.
!template-end!
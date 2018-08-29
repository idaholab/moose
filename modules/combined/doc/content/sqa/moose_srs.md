!sqa load template=system_requirement_specification.md.template
project=MOOSE

!sqa! item key=system-purpose
The !!acro MOOSE!! is a tool for solving complex coupled
Multiphysics equations using the finite element method. !!acro MOOSE!! uses an object-oriented design
to abstract data structure management, parallelism, threading and compiling while providing an easy
to use interface targeted at engineers that may not have a lot of software development
experience. !!acro MOOSE!!  will require extreme scalability and flexibility when compared to other
FEM frameworks. For instance, !!acro MOOSE!! needs the ability to run extremely complex material
models, or even third-party applications within a parallel simulation without sacrificing
parallelism. This capability is in contrast to what is often seen in commercial packages, where
custom material models can limit the parallel scalability, forcing serial runs in the most severe
cases. When comparing high-end capabilities, many !!acro MOOSE!! competitors target modest-sized
clusters with just a few thousand processing cores. !!acro MOOSE!!, however, will be required to
routinely executed on much larger clusters with scalability to clusters available in the top 500
systems ([top500.org](http://www.top500.org)). !!acro MOOSE!! will also be targeted at smaller systems
such as high-end laptop computers.

The design goal of !!acro MOOSE!! is to give developers ultimate control over their physical models
and applications. Designing new models or solving completely new classes of problems will be
accomplished by writing standard C++ source code within the framework's class hierarchy. Scientists
and engineers will be free to implement completely new algorithms using pieces of the framework where
possible, and extending the framework's capabilities where it makes sense to do so. Commercial
applications do not have this capability, and instead opt for either a more rigid parameter system or
a limited application-specific metalanguage.
!sqa-end!

!sqa! item key=system-scope
!!acro MOOSE!!'s scope is to provide a set of interfaces for building !!acro FEM!!
simulations. Abstractions to all underlying libraries are provided.

Solving coupled problems where competing physical phenomena impact one and other in a significant
nonlinear fashion represents a serious challenge to several solution strategies. Small perturbations
in strongly-coupled parameters often have very large adverse effects on convergence behavior. These
adverse effects are compounded as additional physics are added to a model. To overcome these
challenges, !!acro MOOSE!! employs three distinct yet compatible systems for solving these types of
problems.

First, an advanced numerical technique called the !!acro JFNK!! method is
employed to solve the most fully-coupled physics in an accurate, consistent way. An example of this
would be the effect of temperature on the expansion or contraction of a material. While the
!!acro JFNK!!  numerical method is very effective at solving fully-coupled equations, it can also be
computationally expensive. Plus, not all physical phenomena in a given model are truly coupled to one
another. For instance, in a reactor, the speed of the coolant flow may not have any direct effect on
the complex chemical reactions taking place inside the fuel rods.  We call such models
"loosely-coupled". A robust, scalable system must strike the proper balance between the various
modeling strategies to avoid performing unnecessary computations or incorrectly predicting behavior
in situations such as these.

!!acro MOOSE!!'s Multiapp system will allow modelers to group physics into logical categories where
!!acro MOOSE!! can solve some groups fully-coupled and others loosely-coupled. The Multiapp system
goes even further by also supporting a "tightly-coupled" strategy, which falls somewhere between the
"fully-coupled" and "loosely-coupled" approaches. Several sets of physics can then be linked together
into logical hierarchies using any one of these coupling strategies, allowing for several potential
solution strategies. For instance, a complex nuclear reactor model might consist of several
tightly-coupled systems of fully-coupled equations.

Finally, !!acro MOOSE!!'s Transfers system ties all of the physics groups contained within the
Multiapp system together and allows for full control over the flow of information among the various
groups. This capability bridges physical phenomena from several different complementary scales
simultaneously. When these three !!acro MOOSE!! systems are combined, myriad coupling combinations
are possible. In all cases, the !!acro MOOSE!! framework handles the parallel communication, input,
output and execution of the underlying simulation. By handling these computer science tasks, the
!!acro MOOSE!! framework keeps modelers focused on doing research.

!!acro MOOSE!! innovates by building advanced simulation capabilities on top of the very best
available software technologies in a way that makes them widely accessible for innovative
research. !!acro MOOSE!! is equally capable of solving small models on common laptops and the very
biggest FEM models ever attempted---all without any major changes to configuration or source
code. Since its inception, the !!acro MOOSE!! project has focused on both developer and computational
efficiency. Improved developer efficiency is achieved by leveraging existing algorithms and
technologies from several leading open-source packages. Additionally, !!acro MOOSE!! uses several
complementary parallel technologies (both the distributed-memory message passing paradigm and
shared-memory thread-based approaches are used) to lay an efficient computational foundation for
development. Using existing open technologies in this manner helps the developers reduce the scope of
the project and keeps the size of the !!acro MOOSE!! code base maintainable. This approach provides
users with state-of-the-art finite element and solver technology as a basis for the advanced coupling
and solution strategies mentioned previously.

!!acro MOOSE!!'s developers work openly with other package developers to make sure that cutting-edge
technologies are available through !!acro MOOSE!!, providing researchers with competitive research
opportunities. !!acro MOOSE!! maintains a set of objects that hide parallel interfaces while exposing
advanced spatial and temporal coupling algorithms in the framework.  This accessible approach places
developmental technology into the hands of scientists and engineers, which can speed the pace of
scientific discovery.
!sqa-end!


!sqa! item key=user-characteristics

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

!sqa-end!


!sqa! item key=assumptions-and-dependencies
The software should be designed with the fewest possible constraints. Ideally the software should run
on a wide variety of evolving hardware so it should follow well-adopted standards and guidelines. The
software should run on any !!acro POSIX!! compliant system. The software will also make use FEM and
numerical libraries that run on !!acro POSIX!! systems as well. The main interface for the software
will be command line based with no assumptions requiring advanced terminal capabilities such as
coloring and line control.
!sqa-end!

!sqa item key=definitions-and-acronyms

!sqa! item key=definitions
- +Verification+: (1) The process of: evaluating a system or component to determine whether the
  products of a given development phase satisfy the conditions imposed at the start of that
  phase. (2) Formal proof of program correctness (e.g., requirements, design, implementation reviews,
  system tests) [citep:ISO-systems-software].
!sqa-end!

!sqa! item key=acronyms
!acronym list
!sqa-end!


!sqa! item key=system-requirements
!alert construction
The creation of the requirements for !!acro MOOSE!! is an ongoing progress. The
[#functional-requirements] are being generated from source code and soon all requirements should
follow this format.
!sqa-end!

!sqa! item key=minimum-requirements
- A !!acro POSIX!! compliant Unix including the two most recent versions of MacOS and most current
  versions of Linux.
- 4 GB of RAM for optimized compilation (8 GB for debug compilation), 2 GB per core execution
- 100 GB disk space
- C++11 compatible compiler (GCC, Clang, or Intel)
- Python 2.6+
- Git
!sqa-end!

!sqa! item key=functional-requirements
!sqa requirements link=False
!sqa-end!

!sqa! item key=useability-requirements
!sqa requirements-matrix prefix=U
- The system will be command-line and input file driven.
- The system shall return usage messages when unidentified arguments or incorrectly used arguments are passed.
- The system shall provide diagnostics when the input file fails to parse, or the format is incorrect.
- The system will provide on screen information about the simulation and performance characteristics of the solves under normal operating conditions.
!sqa-end!

!sqa! item key=performance-requirements
!sqa requirements-matrix prefix=P
- The system will support multi-process distributed memory execution.
- The system will support multi-process shared memory execution.
- The system will support execution on Unix-based laptops.
- The system will support execution on Unix-based workstation systems.
- The system will support execution on large Unix-based cluster systems.
!sqa-end!

!sqa! item key=system-interfaces
!sqa requirements-matrix prefix=S
- The system shall support POSIX compliant systems.
- The system shall support the Message Passing Interface (MPI) standard.
- The system shall support POSIX ``pthreads''.
- The system shall support Intel Threaded Building Blocks (TBB) interface.
- The system shall support the OpenMP threading interface.
!sqa-end!



!sqa! item key=maintainability
- The latest working version (defined as the version that passes all tests in the current regression
  test suite) shall be publicly available at all times through the repository host provider.
- Flaws identified in the system shall be reported and tracked in a ticket or issue based system. The
  technical lead will determine the severity and priority of all reported issues and assign resources
  at his or her discretion to resolve identified issues.
- The software maintainers will entertain all proposed changes to the system in a timely manner
  (within two business days).
- The core framework in its entirety will be made publicly available under the !!acro LGPL!!
  version 2.0 license.
!sqa-end!


!sqa item key=reliability
The regression test suite will cover at least 80% of all lines of code at all times. Known
regressions will be recorded and tracked (see [#maintainability]) to an independent and
satisfactory resolution.

!sqa item key=information-management
The core framework in its entirety will be made publicly available on an appropriate repository
hosting site. Backups and security services will be provided by the hosting service.

!sqa item key=verification
The regression test suite will employ several verification tests using comparison against known
analytical solutions, the method of manufactured solutions, and convergence rate analysis.

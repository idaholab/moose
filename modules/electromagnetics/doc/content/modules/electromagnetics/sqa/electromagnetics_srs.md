!template load file=sqa/app_srs.md.template app=Electromagnetics category=electromagnetics

!template! item key=system-purpose
!! system-purpose-begin
The [!ac](MOOSE) Electromagnetics Module (hereafter referred to simply as "Electromagnetics application"
or EMM) provides an interface to and library containing Maxwell's equations with the [!ac](MOOSE)
application ecosystem. It is intended to be used as either a standalone simulation code for electrodynamics
or coupled to other [!ac](MOOSE) ecosystem codes (including MOOSE-wrapped applications). Thus, the
EMM uses the same object-oriented design as [!ac](MOOSE) in order to make simulation design and new
development straightforward for engineers and researchers.
!! system-purpose-finish
!template-end!

!template! item key=system-scope
!! system-scope-begin
The scope of the EMM is to provide a set of interfaces and objects for building electrodynamics simulations
based on the finite element method (FEM). Regarding solvers, meshing libraries, as well as solution/coupling
methods and interfaces, the EMM relies on the infrastructure provided by the [!ac](MOOSE) framework.

The system contains, generally, a base set of kernels, boundary conditions, and interface conditions
designed for the solution of vector fields derived from Maxwell's equations. Further, it currently
contains more specific capability in the following general areas:

- Wave reflection, transmission, and absorption
- Electrostatic contact on electrically imperfect surfaces

EMM developers work with framework and other module and application developers to ensure that the EMM
provides adequate capability to support on-going and prospective research opportunities involving aspects
of electromagnetics.
!! system-scope-finish
!template-end!

!template! item key=user-characteristics
Like [!ac](MOOSE), there are three kinds of users working on the EMM:

- +EMM Developers+: These are the core developers of the EMM. They are responsible for following and
  enforcing the software development standards of the module, as well as designing, implementing, and
  maintaining the software.
- +Developers+: A scientist or engineering that uses the EMM alongside [!ac](MOOSE) to build their own
  application. This user will typically have a background in modeling or simulation techniques (and
  perhaps numerical analysis) but may only have a limited skillset when it comes to code development
  using the C++ language. This is the primary focus group of the module. In many cases, these developers
  will be encouraged to contribute module-appropriate code back to the EMM, or to [!ac](MOOSE) itself.
- +Analysts+: These are users that will run the code and perform analysis on the simulations they perform.
  They may interact with developers to request new features and report discovered bugs, and they will
  make heavy use of the input file format.
!template-end!

!template! item key=assumptions-and-dependencies
The EMM is designed with the fewest possible constraints on hardware and software. For more context
on this point, the EMM SRS defers to the framework [framework_srs.md#assumptions-and-dependencies].
Any physics-based assumptions in code simulations and code objects are highlighted in their respective
documentation pages.
!template-end!

!template! item key=reliability
The regression test suite will cover at least 95% of all lines of code within the EMM at all times.
Known regressions will be recorded and tracked (see [#maintainability]) to an independent and satisfactory
resolution.
!template-end!

!template! item key=information-management
The core framework and all modules in their entirety will be made publicly available on an appropriate
repository hosting site. Day-to-day backups and security services will be provided by the hosting service.
More information about MOOSE backups of the public repository on [!ac](INL)-hosted services can be found
on the following page: [sqa/github_backup.md]
!template-end!

!template! item key=policies-and-regulations
!include framework_srs.md start=policies-and-regulations-begin end=policies-and-regulations-finish
!template-end!

!template! item key=packaging
No special requirements are needed for packaging or shipping any media containing [!ac](MOOSE) and
EMM source code. However, some [!ac](MOOSE)-based applications that use the EMM may be export-controlled,
in which case all export control restrictions must be adhered to when packaging and shipping media.
!template-end!

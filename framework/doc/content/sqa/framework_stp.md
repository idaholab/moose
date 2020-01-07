!template load file=stp.md.template category=framework project=Framework

!template! item key=test-scope
The scope of this plan is to outline the necessary steps for tagging a repository
release suitable for deployment and long term support. While much of the testing
for the [!ac](MOOSE) and MOOSE-based applications is automated, [!ac](NQA-1) guidelines
require reviews and approvals for each official supported release suitable for
deployment. This plan describes how automated testing fulfills many of the
quality testing requirements of the software and how the developers and asset
owners can leverage this information when certifying each release.

[!ac](MOOSE) and MOOSE-based software is built into executables that can linked
to other capability extension libraries, or in special cases, testing libraries
that assist in unit testing. All testing in MOOSE and MOOSE-based codes is
of the "dynamic" kind where execution occurs and then results are observed in
some fashion or another. See the [#test-objectives] section for details on the
various types of testing performed.

### Background

[!ac](MOOSE) and MOOSE-based applications use agile development methods, which
accomplish several goals: first, up-front testing helps developers exercise new code
as it's written and helps ensure that existing code isn't impacted in adverse ways.
The up-front testing also greatly reduces the burden of developing test cases just
prior to the release of the software. Finally, by making test development part of
the normal development process, many of the steps of creating quality software are
automatic and natural to the developers on the project.

Most of the testing for release is performed automatically on a daily basis by
[!ac](MOOSE)'s companion continuous integration software called [!ac](CIVET) for
each potential candidate release. After automated test has successfully completed,
an automated merge is made into the application's "stable" branch, which has
undergone extensive testing on multiple platforms, multiple compilers, and often
under various configurations and execution strategies. Each one of these versions
or commits on the stable branch could potentially be adopted for a supported deployment
release.
!template-end!

!template! item key=test-objectives
There are several objectives sought in the testing of the [!ac](MOOSE) framework
and MOOSE-based applications. Those objectives fall into five categories:
unit testing, numerical verification, output behaviors, performance, and error testing.
Thorough testing of all of these categories provides the asset owners the necessary
confidence that the software will perform as expected in a deployment scenario as
different end-user defined inputs are used beyond the built-in [!ac](MOOSE) test cases.
Each of these categories covers an important set of attributes for testing
numerical simulation software. These category definitions are provided here:

- +unit testing+: Testing individual object instances, methods or functions for expected
  behaviors outside of the normal usage within the system.
- +numerical verification+: Running a simulation and verifying numerical results against
  an expected result (This is a looser definition than is typically used for verification and
  validation).
- +output behaviors+: Testing that the application outputs specific information to the screen
  or file system.
- +performance+: Running a simulation, typically on multiple processors, and verifying
  expected scaling.
- +error testing+: Running a simulation with bad inputs and testing for expected warnings,
  errors, or exceptions.

!template-end!

!template! item key=assumptions
[!ac](MOOSE) and MOOSE-based software is assumed to be dynamically-linked command-line
UNIX (POSIX) compatible executables built on the target system. Being HPC software, and
the fact that our normal configuration relies on shared-libraries. It generally not
advisable to build MOOSE on one system and execute it on another system (with the exception
of a homogeneous cluster environment).

MOOSE is assumed to be stateless, reading all inputs from local or network mounted file-systems.
When deployed for parallel testing or use, standard MPI networking is expected to function
among cluster compute nodes. MOOSE does not require any special file system (i.e. parallel
file systems), however high performance file systems can improve performance of large
simulations and also the speed at which the automated testing system can launch, run, and
inspect test results.
!template-end!

!template item key=constraints
[!ac](MOOSE) is designed to be built and tested in-situ on the end-use machine. There is
no requirement for separate testing or acceptance environments. As each independent invocation
of a MOOSE-based simulation maintains its own environment. Acceptance testing may be performed
at full-scale provided resources are available. Therefore there are no constraints on
testing of MOOSE-based software.

!template! item key=test-types

The [!ac](MOOSE) regression test suite automatically covers all necessary "unit"
and "system" test cases built by developers, and approved by independent reviewers during
the development of the application. These tests include bug fixes and other test cases
that are added to the suite as errors are reported, patched, and subsequently deployed.

Since MOOSE is designed to be [!ac](HPC) software, limited performance testing is also
part of the suite but is limited in scope and size to be part of the automated testing
system. Applications may augment their standard test suite with additional layers of
testing, both automated and manual to assist in testing their software for deployment.
These additional tests fall with these types: "performance", "integration", and "acceptance".

- +unit+: Testing individual object instance, methods or function for expected
  behaviors outside of the normal usage within the system, but within the public [!ac](API).
- +system+: Testing performed using the standard application executable with
  an input file designed to verify a specific behavior.
- +performance+: (where applicable) Testing performed using the standard application binary
  verifying the execution speed and scalability of the application on multiple cores
  for sufficiently sized simulations.
- +integration+: (where applicable) Testing performed using the standard application that
  is composed of one or more application libraries.
- +acceptance+: (where applicable) Testing performed using the standard application binary verifying
  specific end-use assessment or validation cases.

Integration testing is intended to cover compound applications composed of multiple
sub-applications. Examples would include a full-core simulator consisting of neutron
transport application, a fuels performance application, and a thermal-hydraulics
simulation.

Acceptance testing can include anything required by the end-user of an application.
Examples may include assessment testing indicating the ability for the software to
produce a solution for industry standard benchmarks, or test cases. Parallel performance
on a cluster computing system might also be included as part of the acceptance suite.


!template-end!

!template! item key=approval-requirements

Unit and system test cases are generally created during the development of new components
of the software or as part of a bug fix to address an error reported during the use of
the software. These test cases are reviewed and approved by independent reviewers of the
change control board. Additional test cases in the areas of performance, integration,
and acceptance are generally are at the discretion of the technical lead, IT Project or
M&O Manager.

| Activity | Authorized Role |
| :- | :- |
| Unit Test Case Reviewer(s):        | Independent Reviewer |
| System Test Case Reviewer(s):      | Independent Reviewer |
| Performance Test Case Reviewer(s): | Technical Lead, Independent Reviewer |
| Integration Test Case Reviewer(s): | Technical Lead, IT Project or M&O Manager |
| Acceptance Test Case Reviewer(s):  | Technical Lead, IT Project or M&O Manager |
| --                                 | -- |
| Unit Result Reviewer:           | Independent Reviewer |
| System Result Reviewer:         | Independent Reviewer |
| Performance Result Reviewer:    | Independent Reviewer |
| Integration Result Reviewer:    | Independent Reviewer |
| Acceptance Result Reviewer:     | Independent Reviewer |
| --                              | -- |
| Unit Result Approver:           | Technical Lead |
| System Result Approver:         | Technical Lead |
| Performance Result Approver:    | Technical Lead |
| Integration Result Approver:    | Technical Lead |
| Acceptance Result Approver:     | Technical Lead |

!template-end!

!template item key=test-iteration
[!ac](MOOSE) and MOOSE-based software is stateless, deterministic software for a given input.
Therefore, a single testing iteration on each identified configuration is sufficient for
completing the required tests necessary for deployment.

!template! item key=test-automation
[!ac](MOOSE) and MOOSE-based software rely heavily on full test automation. Since each application
is stateless and completely command-line driven, developing a thorough test suite is generally
more straightforward for MOOSE than it is for other business system type software. The MOOSE
repository includes a general-purpose, extendable "Test Harness", which is heavily leveraged
to run the MOOSE test suite in every location where MOOSE-based software is deployed. The
Test Harness is used throughout all phases of MOOSE development from initial development, the
change request process, deployment testing, and finally end-use in-situ testing. The Test Harness
is even suitable for testing on large deployment clusters and supports the "PBS" queuing system.

The extensible nature of the Test Harness ensures that that it can be used by MOOSE-based
applications that with testing scope that extends beyond that supplied with the MOOSE framework.
However, the Test Harness includes an extensive suite of "Tester" types that cover each
of the identified test types identified in [#test-types]. A complete list of the built-in
Testers is included here:

!media media/sqa/testers.svg
       id=testers
       caption=Tester class diagram

- +RunApp+: A tester designed to assemble common command line arguments for executing
  MOOSE-based applications including launching with MPI or threads.
- +RunCommand+: A generic tester that can execute an arbitrary command.
- +FileTester+: An intermediate base class that runs a MOOSE-based command and expects
  to process a file written out by the MOOSE-based simulation.
- +RunException+: A tester that expects the application to produce a warning or error
  based on bad inputs, missing files, permissions, etc.
- +CheckFiles+: A tester that looks for the creation of specific files after a simulation
  runs without regard for the contents of those files.
- +Exodiff+: Compares "ExodusII" format files output by the simulation to those checked
  into the repository as the "gold" standard for a given test within numeric tolerances.
- +CSVDiff+: Compares "CSV" format files output by the simulation to those checked
  into the repository as the "gold" standard for a given test within numeric tolerances.
- +ImageDiff+: Compares various image format files output by the simulation to those checked
  into the repository as the "gold" standard for a given test within tolerance.
- +VTKDiff+: Compares "VTK" format files output by the simulation to those checked
  into the repository as the "gold" standard for a given test within numeric tolerances.
- +JacobianTester+: Appends additional arguments to the command line to trigger special solver
  modes in MOOSE for the purpose of producing "finite-difference" Jacobians for which to compare
  the Jacobians produces by the simulation.

For each of these tester types. The Test Harness is able to execute the application with
a developer designed input and verify the correct result automatically.
!template-end!

!template item key=human-resources
Deployment testing for [!ac](MOOSE) and MOOSE-based applications requires minimal
human resources. A system engineer is required to ensure the proper end-user environment
is setup with proper system prerequisites. An independent-reviewer is then needed to
execute any additional test not already verified by the automated test system CIVET and
to manually inspect those results for accuracy prior to deployment.

!template! item key=hardware-software-resources
If a specific end-user environment is required by a customer, those specs must be
supplied to the system engineer to prepare that environment. Alternatively, if remote
access is available to the end-user system. The system engineer may be granted proper
permissions to assist in setting up the environment on the customer's system.

If no specific customer is required for a specific release. [!ac](MOOSE) and MOOSE-based software
will be tested under the standard supported build system configuration(s). These systems
are generally modern Linux and Mac distributions with recent compiler stacks available. Specific
information on the current environments is always stored and available in the
"idaholab/package_builder" repository (privately maintained for internal INL use).
!template-end!

!template item key=services-applications
[!ac](MOOSE) and MOOSE-based software generally does not require any additional resources
beyond the end-use deployment system once the software is installed. During installation
either an Internet connection or media containing the software must be available to install
the software. Internet connectivity is not required after installation on the end-use system.

!template! item key=task-responsibilities

| Tasks | Responsibility |
| :- | :- |
| 1. Complete programming and test case(s) | Developer |
| 2. Test resources creation (mesh, input files, etc.) | Developer |
| 3. Set up test environment | System engineer |
| 4. Review automated test cases | Independent reviewer |
| 5. Notify developers of failures | Independent reviewer |
| 6. Review and approve final results of the test | Independent Reviewer, Technical Lead |

!template-end!

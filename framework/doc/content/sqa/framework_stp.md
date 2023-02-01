!template load file=stp.md.template category=framework project=Framework

!template! item key=test-scope
This plan details the testing implemented for the development of MOOSE or MOOSE-based application
that leads to a stable revision, which upon a secondary review by the Project Lead can be
released. The [!ac](NQA-1) standard necessitates reviews and approvals for each release. This plan
describes how automated testing fulfills obligatory testing of the software and how the Project Lead
can leverage this information when performing a release.

[!ac](MOOSE) or MOOSE-based applications use an agile development method that tests all proposed
changes prior to integration into the main repository. This allows developers to exercise proposed
changes as they are written and ensure that existing code is not impacted in adverse ways. Testing is
an integral part of the normal development process, as such the necessary testing and reviews to
satisfy the [!ac](NQA-1) standard are natural for contributors to the project.

All testing performed is "dynamic" that attempt to identify defects by executing the software. All
testing for each revision is performed automatically using [!ac](CIVET). After automated testing has
successfully completed and a technical review is performed, an automated merge is made into a
"stable" revision. Each revision is eligible for release at the discretion of the Project Lead and
subject to a complete release review.
!template-end!

!template! item key=test-objectives
All test types, as detailed in [#test-types], for [!ac](MOOSE) or MOOSE-based application have a
single objective: that when executed with the prescribed input the software shall produce the
expected output. The type of test indicates the type of output that shall be considered, which
can range from numerical verification to error testing. The overall object is to provide the
necessary confidence that the software will perform as expected for each for the defined test
cases to ensure that the software properly handles abnormal conditions and events as well as credible
failures, does not perform adverse unintended functions, and does not degrade the system either by
itself, or in combination with other functions.

By the nature of the software as a library it is not possible to guarantee the functionality
of the software from an end-user perspective, since the input provided by the user cannot be
controlled.
!template-end!

!template! item key=assumptions
[!ac](MOOSE) and MOOSE-based applications are assumed to be dynamically-linked command-line
UNIX (POSIX) compatible executable built on the target system. Being [!ac](HPC) software, and
the fact that our normal configuration relies on shared-libraries, it is generally not
advisable to build [!ac](MOOSE) on one system and execute it on another system (with the exception
of a homogeneous cluster environment).

[!ac](MOOSE) and MOOSE-based applications are assumed to be stateless, reading all inputs from local
or network mounted file-systems.  When deployed for parallel testing or use, standard [!ac](MPI)
networking is expected to function among cluster compute nodes. [!ac](MOOSE) or MOOSE-based
applications does not require any special file system (i.e., parallel file systems), however high
performance file systems can improve performance of large simulations and also the speed at which the
automated testing system can launch, run, and inspect test results.
!template-end!

!template item key=constraints
[!ac](MOOSE) and MOOSE-based applications are designed to be built and executed in-situ on the
end-use machine. There is no requirement for separate testing or acceptance environments, as each
independent invocation of a simulation maintains its own environment. Acceptance testing
may be performed at full-scale provided resources are available. Therefore, there are no constraints
on testing of [!ac](MOOSE) and MOOSE-based applications.

!template! item key=test-types
It is possible to categorize test cases in many ways such as "system", "integration", "performance",
or "acceptance" testing. [!ac](MOOSE) and MOOSE-based applications do not categorize test cases;
they are simply defined and executed as a complete set and automatically executed as defined in
[#test-automation].

### Required tests and test sequence

All tests defined within [!ac](MOOSE) or MOOSE-based application must be executed and pass for
all revisions and releases of the software. All test cases within one test specification (a "tests"
file) are expected to run in the order defined. The specifications may be executed in any order.

### Required Ranges of input parameters

Test cases are created by contributors during the change control process. The independent reviewer
is responsible for ensuring that input parameters are tested across the expected operational ranges
to the extent necessary for the proposed change.

### Identification of the stages at which testing is required

Testing for [!ac](MOOSE) and MOOSE-based applications shall include the levels of testing as defined
in [fig:civet_flow]. The testing is automated to the extent possible and the "Next" and "Devel" branch
testing may be combined at the discretion of the application.

!media civet_flow.png id=fig:civet_flow caption=Required stages for testing of [!ac](MOOSE), MOOSE-based
                         applications, and {{app}}. The "Next" and "Devel" branch testing may be combined
                         at the discretion of the application.

### Requirement for testing logic branches

Test cases are created by contributors during the change control process. The independent reviewer is
responsible for ensuring that all logical code paths are tested to the extent necessary for the
proposed change.

### Requirements for hardware integration

The hardware and software configurations tested for [!ac](MOOSE) and MOOSE-based applications are at
the discretion of the Project Lead. Upon release the hardware and software configurations utilized
shall be included in the release.

### Anticipated output values

The anticipated output values of each test cases are defined within each test case. The format of the
output is dictated by the "Tester" as detailed in [#test-automation].

### Acceptance criteria

All tests have a pass/fail acceptance criteria based on the anticipated output as dictated by the
"Tester", see [#test-automation]. If the execution output matches the anticipated output than the
test is accepted (pass), otherwise it is rejected (fail).

In addition, test coverage reports will be created for all proposed changes. Ideally, the overall line
coverage should increase or remain constant at the discretion of the reviewer, but coverage should
never drop below the level of 80%. Additionally, the independent reviewer is expected to use the
coverage reports to ensure that the proposed changes are tested at an appropriate level.

### Reports, records, standard formatting, and conventions

Each test case shall report the acceptance status (pass or fail). If the test case fails the
reason for the failure shall be included. The information reported is dependent on the "Tester",
see [#test-automation] for details.
!template-end!

!template item key=approval-requirements
!! approval-requirements-begin
All test cases are created by contributors during the development process and approved by independent
reviewer. The creation of the test cases follows the change control process. These test cases can
be in response to bug fixes or as a part of an enhancement.
!! approval-requirements-end


!template item key=test-iteration
[!ac](MOOSE) and MOOSE-based applications are stateless, deterministic software for a given input.
Therefore, a single testing iteration on each identified configuration is sufficient for
completing the required tests necessary for deployment.

!template! item key=test-automation
[!ac](MOOSE) and MOOSE-based applications rely heavily on full test automation. Since each
application is stateless and command-line driven, developing a thorough test suite is generally more
straightforward than it is for other business system type software. The MOOSE repository includes a
general-purpose, extendable "Test Harness", which is heavily leveraged to run the test cases. The
Test Harness is used throughout all phases of MOOSE development from initial development, the change
request process, deployment testing, and finally end-use in-situ testing. The Test Harness is even
suitable for testing on large deployment clusters and supports the "PBS" queuing system.

The Test Harness includes a suite of "Tester" types to enable complete testing of [!ac](MOOSE) and
MOOSE-based applications. For each of the types, the Test Harness is able to execute the application
with a developer designed input and verify the correct result automatically.  A complete list of the
built-in Testers is included here:

!! testers-begin

+RunApp+\\
A tester designed to assemble common command line arguments for executing MOOSE-based application
including launching with MPI or threads.\\
*Anticipated Output*: A return code of 0 or 1.\\
*Acceptance Criteria*: A non-zero return code is acceptable (pass).\\
*Reports and Records*: The return code and the system standard output returned from the application.

+RunCommand+\\
A generic tester that can execute an arbitrary command.\\
*Anticipated Output*: A return code of 0 or 1.\\
*Acceptance Criteria*: A non-zero return code is acceptable (pass).\\
*Reports and Records*: The return code and the system standard output returned from the command.

+FileTester+\\
An intermediate base class that runs a MOOSE-based command and expects to process a file written out
by the MOOSE-based simulation.\\
*Anticipated Output*: Files created as a result of the execution of an application.\\
*Acceptance Criteria*: If the files are created the result is acceptable (pass).\\
*Reports and Records*: This list of files missing upon failure

+RunException+\\
A tester that expects the application to produce a warning or error based on bad inputs, missing
files, permissions, etc.\\
*Anticipated Output*: A non-zero exist status and an error message.\\
*Acceptance Criteria*: If the exit status is non-zero and the error message is present the result is acceptable (pass).\\
*Reports and Records*: The return code and the system standard output returned from the application.

+CheckFiles+\\
A tester that looks for the creation of specific files after a simulation runs without regard for the
contents of those files.\\
*Anticipated Output*: Files created with the expected content.\\
*Acceptance Criteria*: If the files are created the desired content exists then result is acceptable (pass).\\
*Reports and Records*: This list of files created an the missing content.

+Exodiff+\\
Compares "ExodusII" format files output by the simulation to those checked into the repository as the
"gold" standard for a given test within numeric tolerances.\\
*Anticipated Output*: ExodusII files created as a result of the execution of an application.\\
*Acceptance Criteria*: If the files created match a known "gold" standard the result is acceptable (pass).\\
*Reports and Records*: Upon failure, a report detailing the differences in the files.

+CSVDiff+:\\
Compares "CSV" format files output by the simulation to those checked into the repository as the
"gold" standard for a given test within numeric tolerances.\\
*Anticipated Output*: CSV files created as a result of the execution of an application.\\
*Acceptance Criteria*: If the files created match a known "gold" standard the result is acceptable (pass).\\
*Reports and Records*: Upon failure, a report detailing the differences in the files.

+ImageDiff+\\
Compares various image format files output by the simulation to those checked into the repository as
the "gold" standard for a given test within tolerance.\\
*Anticipated Output*: Image files created as a result of the execution of an application.\\
*Acceptance Criteria*: If the files created match a known "gold" standard the result is acceptable (pass).\\
*Reports and Records*: Upon failure, a report detailing the differences in the files.

+VTKDiff+\\
Compares "VTK" format files output by the simulation to those checked into the repository as the
"gold" standard for a given test within numeric tolerances.\\
*Anticipated Output*: VTK files created as a result of the execution of an application.\\
*Acceptance Criteria*: If the files created match a known "gold" standard the result is acceptable (pass).\\
*Reports and Records*: Upon failure, a report detailing the differences in the files.

+JacobianTester+\\
Appends additional arguments to the command line to trigger special solver modes for the purpose of
producing "finite-difference" Jacobians for which to compare the Jacobians produces by the
simulation.\\
*Anticipated Output*: A return code of 0 or 1.\\
*Acceptance Criteria*: If the simulation values match the finite-difference the result is acceptable (pass).\\
*Reports and Records*: Upon failure, a report detailing the differences between the Jacobians.

+PythonUnitTest+\\
A tested designed to run Python scripts within the MOOSE test suite, generally containing unit tests.\\
*Anticipated Output*: A script return code of 0 or 1.\\
*Acceptance Criteria*: A 0 return code (meaning the script completed successfully) is acceptable (pass).\\
*Reports and Records*: Upon failure, a report detailing the failures experienced within the test script.

!! testers-end
!template-end!

!template item key=human-resources
Testing for [!ac](MOOSE) and MOOSE-based applications requires minimal human resources. A system engineer
is required to ensure the proper end-user environment is setup with proper system prerequisites. The
Project Lead is should verify that the automated test system operated correctly prior to release.

!template! item key=hardware-software-resources
If a specific end-user environment is required by a customer, those specifications must be
supplied to the system engineer to prepare that environment. Alternatively, if remote
access is available to the end-user system, the system engineer may be granted proper
permissions to assist in setting up the environment on the customer's system.

If no specific customer is required for a specific release, [!ac](MOOSE) and MOOSE-based applications
will be tested under the standard supported build system configuration(s). These systems
are generally modern Linux and macOS distributions with recent compilers. Specific
information on the tested environments for a release is stored in the release.
!template-end!

!template item key=services-applications
[!ac](MOOSE) and MOOSE-based applications generally does not require any additional resources
beyond the end-use system once the software is installed. During installation
either an Internet connection or media containing the software must be available to install
the software. Internet connectivity is not required after installation on the end-use system.

!template! item key=task-responsibilities
!! task-responsibilities-begin
The creation and execution of the test cases is part of the change control process, as such the
associated roles and reponsibilties are minimal.

| Task | Responsibility | Role |
| :- | :- | :- |
| 1. | Complete programming and test case(s) | Contributor |
| 2. | Review test cases and automated results | Independent reviewer |
| 3. | Review and approve final results for release | Project lead |

!! task-responsibilities-end
!template-end!

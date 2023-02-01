!template load file=sqa/stp.md.template project=MOOSE Tools

!template! item key=test-scope
This plan details the testing implemented for the development of [!ac](MOOSE) Tools that leads to a
stable revision, which upon a secondary review by the Project Lead can be released. The [!ac](NQA-1)
standard necessitates reviews and approvals for each release. This plan describes how automated testing
fulfills obligatory testing of the software and how the Project Lead can leverage this information
when performing a release.

[!ac](MOOSE) Tools uses an agile development method that tests all proposed changes prior to integration into
the main repository. This allows developers to exercise proposed changes as they are written and
ensure that existing code is not impacted in adverse ways. Testing is an integral part of the normal
development process, as such the necessary testing and reviews to satisfy the [!ac](NQA-1) standard
are natural for contributors to the project.

All testing performed is "dynamic" that attempt to identify defects by executing the software. All
testing for each revision is performed automatically using [!ac](CIVET). After automated testing has
successfully completed and a technical review is performed, an automated merge is made into a
"stable" revision. Each revision is eligible for release at the discretion of the Project Lead and
subject to a complete release review.
!template-end!

!template! item key=test-objectives
All test types, as detailed in [#test-types], for [!ac](MOOSE) Tools have a single objective: that when executed
with the prescribed input the software shall produce the expected output. The type of test indicates
the type of output that shall be considered, which can range from numerical verification to error
testing. The overall object is to provide the necessary confidence that the software will perform as
expected for each of the defined test cases to ensure that the software properly handles abnormal
conditions and events as well as credible failures, does not perform adverse unintended functions,
and does not degrade the system either by itself, or in combination with other functions.

By the nature of the software as a library it is not possible to guarantee the functionality
of the software from an end-user perspective, since the input provided by the user cannot be
controlled.
!template-end!

!template! item key=assumptions
[!ac](MOOSE) Tools are assumed to be command-line UNIX (POSIX) compatible Python executables and scripts run on
the target system. The utilities within [!ac](MOOSE) Tools are assumed to be stateless, reading all inputs from
local or network mounted file-systems.  When deployed for parallel testing or use, standard [!ac](MPI)
networking is expected to function among cluster compute nodes. [!ac](MOOSE) Tools does not require any special
file system (i.e., parallel file systems), however high performance file systems can improve performance
of large utilities and also the speed at which the automated testing system can launch, run, and inspect
test results.
!template-end!

!template item key=constraints
[!ac](MOOSE) Tools are designed to be executed in-situ on the end-use machine. There is no requirement for separate
testing or acceptance environments, as each independent invocation of a utility maintains its own
environment. Acceptance testing may be performed at full-scale provided resources are available.
Therefore, there are no constraints on testing of [!ac](MOOSE) Tools.

!template! item key=test-types
It is possible to categorize test cases in many ways such as "system", "integration", "performance",
or "acceptance" testing. [!ac](MOOSE) Tools does not categorize test cases; they are simply defined and executed
as a complete set and automatically executed as defined in [#test-automation].

### Required tests and test sequence

All tests defined within [!ac](MOOSE) Tools utilities must be executed and pass for all revisions and releases
of the software. All test cases within one test specification (a "tests" file) are expected to run in
the order defined. The specifications may be executed in any order.

### Required Ranges of input parameters

Test cases are created by contributors during the change control process. The independent reviewer
is responsible for ensuring that input parameters are tested across the expected operational ranges
to the extent necessary for the proposed change.

### Identification of the stages at which testing is required

Testing for [!ac](MOOSE) Tools shall include the levels of testing as defined in [fig:civet_flow]. The testing
is automated to the extent possible and the "Next" and "Devel" branch testing may be combined at the
discretion of the application.

!media civet_flow.png id=fig:civet_flow caption=Required stages for testing of [!ac](MOOSE), MOOSE-based
                         applications, and [!ac](MOOSE) Tools. The "Next" and "Devel" branch testing may be combined
                         at the discretion of the application.

### Requirement for testing logic branches

Test cases are created by contributors during the change control process. The independent reviewer is
responsible for ensuring that all logical code paths are tested to the extent necessary for the
proposed change.

### Requirements for hardware integration

The hardware and software configurations tested for [!ac](MOOSE) Tools are at the discretion of the Project Lead.
Upon release the hardware and software configurations required shall be included in the release documentation.

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
!include framework_stp.md start=approval-requirements-begin end=approval-requirements-end

!template! item key=test-iteration
[!ac](MOOSE) Tools utilities are stateless, deterministic software for a given input. Therefore, a single testing
iteration on each identified configuration is sufficient for completing the required tests necessary
for deployment.
!template-end!

!template! item key=test-automation
{{MOOSE Tools}} rely heavily on full test automation. Since each utility is stateless and command-line
or [!ac](GUI) driven, developing a thorough test suite is generally more straightforward than it is
for other business system type software. The MOOSE repository includes a general-purpose, extendable
"Test Harness", which is heavily leveraged to run the test cases. The Test Harness is used throughout
all phases of MOOSE development from initial development, the change request process, deployment testing,
and finally end-use in-situ testing. The Test Harness is even suitable for testing on large deployment
clusters and supports the "PBS" queuing system.

The Test Harness includes a suite of "Tester" types to enable complete testing of [!ac](MOOSE) Tools. For each
of the types. The Test Harness is able to execute the application with a developer designed input and
verify the correct result automatically. A complete list of the built-in Testers is included here:

!include framework_stp.md start=testers-begin end=testers-end
!template-end!

!template! item key=human-resources
Testing for [!ac](MOOSE) Tools utilities requires minimal human resources. A system engineer is required
to ensure the proper end-user environment is setup with proper system prerequisites. The Project Lead
should verify that the automated test system operated correctly prior to release.
!template-end!

!template! item key=hardware-software-resources
If a specific end-user environment is required by a customer, those specifications must be
supplied to the system engineer to prepare that environment. Alternatively, if remote
access is available to the end-user system, the system engineer may be granted proper
permissions to assist in setting up the environment on the customer's system.

If no specific customer is required for a specific release, [!ac](MOOSE) Tools will be tested under the standard
supported build system configuration(s). These systems are generally modern Linux and macOS distributions
with recent versions of Python. Specific information on the tested environments for a release is stored
in the release.
!template-end!

!template item key=services-applications
[!ac](MOOSE) Tools generally does not require any additional resources beyond the end-use system once the software
is installed. During installation either an Internet connection or media containing the software must
be available to install the software. Internet connectivity is not required after installation on the
end-use system.

!template item key=task-responsibilities
!include framework_stp.md start=task-responsibilities-begin end=task-responsibilities-end

!template load file=rtm.md.template project=Framework

!template! item key=minimum_requirements
!include sqa/minimum_requirements.md
!template-end!

!template! item key=system-purpose
!include sqa/system_purpose.md
!template-end!

!template! item key=system-scope
!include sqa/system_scope.md
!template-end!

!template! item key=assumptions-and-dependencies
!include sqa/assumptions_and_dependencies.md
!template-end!

!template! item key=pre-test
!! pre-test-begin
Ideally all testing should be performed on a clean test machine following
one of the supported configurations set up by the test system engineer. Testing
may be performed on local workstations and cluster systems containing supported
operating systems.

The repository should be clean prior to building and testing. When using "git"
this can be done by doing a force clean in the main repository and each one
of the submodules:

```
git clean -xfd
git submodule foreach 'git clean -xfd'
```

All tests must pass in accordance with the type of test being performed. This list
can be found in the [Software Test Plan](sqa/framework_stp.md).

!! pre-test-finish
!template-end!

!template item key=functional-requirements
!sqa requirements collections=FUNCTIONAL category=framework

!template item key=usability-requirements
!sqa requirements collections=USABILITY category=framework

!template item key=performance-requirements
!sqa requirements collections=PERFORMANCE category=framework

!template item key=system-interfaces-requirements
!sqa requirements collections=SYSTEM category=framework

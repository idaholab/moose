!template load file=rtm.md.template category=framework project=Framework

!template! item key=minimum_requirements
!include sqa/minimum_requirements.md
- Many flavors of modern Linux and the two most recent versions of Mac OS X
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
Ideally all testing should be performed on a clean test machine following
one of the supported configurations setup by the test system engineer. Testing
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

!template-end!

!template! item key=requirements-traceability-matrix
!sqa requirements category={{category}}
!template-end!

!template! item key=requirement-collections-intro
A "collection" is a grouping of requirements that are serving a similar purpose. For example, the
"FAILURE_ANALYSIS" collection is comprised of requirements that perform checks for simulation
failures. The following table lists the names and descriptions of the available collections.

!sqa collections-list caption=List of requirement "collections" names and descriptions.

The following is a complete list of each requirement that has been assigned to a collection.
!template-end!

!template! item key=requirement-collections
!sqa collections category={{category}}
!template-end!

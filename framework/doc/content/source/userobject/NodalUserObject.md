# NodalUserObject

The NodalUserObject is a [UserObject](syntax/UserObjects/index.md) that executes on each node in the domain.
When running in parallel, each processor will execute over a subset of the total nodes so parallel aggregation
may be necessary.

## Executing one or more times on a single node

NodalUserObjects may be made [BlockRestrictable](BlockRestrictable.md). When restricted to
two or more blocks, users may set a parameter in the respective input file block to choose whether or not
MOOSE should "visit" (execute on) nodes on shared block boundaries only once or multiple times.
Default: execute multiple times.

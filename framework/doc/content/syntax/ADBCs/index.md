# ADBCs System

A `BC` is an object that represents a PDE boundary condition. It is applied
to select boundaries of the simulation domain using the `boundary` parameter in
the relevant sub-block of the `ADBCs` block of a MOOSE input file. There are two
different flavors of `BCs`: `IntegratedBCs` and `NodalBCs`. `IntegratedBCs` are
integrated over the domain boundary and are imposed weakly. `NodalBCs` are
applied strongly at individual nodes and are not integrated. An `AD` prefix,
as in `ADNodalBC`, indicates that the Jacobians for subclasses deriving from the
parent type are computed using automatic differentiation. In an `ADBC` subclass the `computeQpResidual()` function +must+ be overridden.

## Custom ADBC Creation

To create a custom `ADNodalBC`, you can follow the pattern of the
[`ADFunctionDirichletBC`](/ADFunctionDirichletBC.md) object implemented and
included in the MOOSE framework. For demonstration of an `ADIntegratedBC`, you
can refer to the [`ADRobinBC`](/ADRobinBC.C) test object.

# SolveObjectExecutor

`SolveObjectExecutor<T>` is a class template. It runs the `solve()` of a `SolveObject` of type
`T` as an executor. The executor fails if `solve()` returns `false`.

## Overview

A `SolveObject` is constructed from an `Executioner` and reads the parameters of that
executioner. `SolveObjectExecutor<T>` owns a `T` that it constructs from itself. So the
parameters of `T` are parameters of the executor, and an input sets them in the executor block.
The executor also calls `T::initialSetup()` in its own `initialSetup()`.

With this executor, a solve object can be an inner executor of other executors, for example of a
[FixedPointExecutor.md].

## Registration

The template has no registered type. An application registers each instantiation that it needs
with a type alias:

```
using FooSolveExecutor = SolveObjectExecutor<FooSolve>;
registerMooseObject("FooApp", FooSolveExecutor);
```

The input then uses `type = FooSolveExecutor`.

## Example input syntax

The test application registers `RichardsonTestSolveExecutor`, which runs a Richardson iteration
on one nonlinear system. Here it solves one of the systems of a multi-system fixed point
iteration:

!listing test/tests/executors/solve_object/richardson.i block=Executors

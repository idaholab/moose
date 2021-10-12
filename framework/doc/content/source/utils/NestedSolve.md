# NestedSolve

## Overview

The `NestedSolve` utility class implements a nonlinear solve for NxN systems.
It can be used in Kernels, Materials, and UserObjects to compute quantities
given by the solution of a nonlinear equation system rather than a closed form
expression.

Such a nonlinear equation system is defined by a residual function and a
Jacobian function. These functions are provided as C++11 lambda expression. A
lambda expression can access all local and member variables from the enclosing
scope. In this application that mainly refers to coupled variable values and
material properties available in the class the NestedSolve is utilized.

### Basic API

`NestedSolve` is a C++ class object needs to be instantiated (constructed) to
set up a solver environment. This solver environment can be a local or member
variable of the object it is used within.

The purpose of the solve environment is to store *solver options*, such as
tolerances, and *solver state*, such as convergence reason or failure state of
the previous solve. A solve environment can be reused for multiple solves.

The main API of `NestedSolve` is the `nonlinear` member function, which exists in two flavors:

1. `NestedSolve::nonlinear(T & guess, L compute)`, which takes a writable reference to a variable if type `T` that contains an initial guess for the solution going into the function, and the solution once the call completes. `compute` is a lambda expression that takes references to the current solution value, the residual, and the Jacobian, and needs to update the latter two.
2. `NestedSolve::nonlinear(T & guess, LR computeResidual, LJ computeJacobian)` behaves largely similar to the first overload, but it takes two separate lambdas for computing the residual and the Jacobian independently. This enables the efficient use of line search and trust region algorithms internally.

### Supported data types

The recommended data types for the solution/residual (`T`) and Jacobian are Eigen's
dynamic size Matrix objects. the `NestedSolve::Value<>` and
`NestedSolve::Jacobian<>` typedefs are available as shortcuts for those types.

Specializations and overloads exist for the `nonlinear` solve API to deal with
the special cases of 1x1 systems (with the solution,residual, and Jacobians
being `Real` scalars), and for 3x3 systems (with the solution and residual bein
`RealVectorValues` and Jacobians being `RankTwoTensor` values). The correct
overload is picked based on the type `T` of the initial `guess` parameter.

### Checking convergence state

The state of a solve can be checked using the `getSolve()` method. This returns
an enumeration describing how the solve converged or if it did not converge. The
enumeration can take the following values:

- `NestedSolve::State::NONE`: No solve has begun. This is the initial value.
- `NestedSolve::State::CONVERGED_ABS`: The solve converged due to meeting an absolute tolerance.
- `NestedSolve::State::CONVERGED_REL`: The solve converged due to meeting a relative tolerance.
- `NestedSolve::State::EXACT_GUESS`: The solve converged due to the initial guess giving a
  zero residual.
- `NestedSolve::State::NOT_CONVERGED`: The solve did not converge.

Note that in the case that both the absolute and relative tolerances being met
simultaneously, the value `NestedSolve::State::CONVERGED_REL` will be returned.

## Example

### Basic usage

We first construct a `NestedSolve` object. This object can be reused for
multiple solves, and could be a member of the class you are using it in.

```
NestedSolve solver;
```

Next we set the solution (vector) type. `NestedSolve::Value<>` is a dynamicly
sized vector class from the Eigen library. Eigen uses the `<<` operator to
initialize such a vector (two components in this case).

```
NestedSolve::Value<> solution(2);
solution << 1.98, 1.02;
```

Next we set a custom relative tolerance. The default value here is 1e-8.

```
solver.setRelativeTolerance(1e-10);
```

Next we define the residual and Jacobian functions. Here we use a single lambda
with three arguments:

1. The first argument is the current guess for the solution and is the _input_ to the functions, it is a vector of length N (or a scalar).
2. Argument two is a writable reference to the residual vector (of the same size as the current guess), the value of which is to be calculated by the lambda.
3. This is followed by the Jacobian matrix of the system, the derivative of the resdual vector with respect to the guess/solution.

```
auto compute = [&](const NestedSolve::Value<> & guess,
                   NestedSolve::Value<> & residual,
                   NestedSolve::Jacobian<> & jacobian) {
  residual(0) = guess(0) + guess(0) * guess(1) - 4;
  residual(1) = guess(0) + guess(1) - 3;

  jacobian(0, 0) = 1 + guess(1);
  jacobian(0, 1) = guess(0);
  jacobian(1, 0) = 1;
  jacobian(1, 1) = 1;
};
```

Note that `jacobian(i,j)` is the derivative of `residual(i)` with respect to `guess(j)`.

Lastly we pass the initial guess along with the residual/Jacobian `compute`
lambda to the `NestedSolve::nonlinear` method.

```
solver.nonlinear(solution, compute);
```

`solution` will now be updated from the initial guess to the actual solution of
the system.

Then to check that the solve was successful, one can do the following:

```
if (solver.getState() == NestedSolve::State::NOT_CONVERGED)
{
  // Take some action for the case of no convergence.
}
```

### Powell's Dogleg method solver

While the previous example used a single lambda to compute residual and Jacobian
at the same time, we can instead change the code to have sparate lambdas to
allow independent calculation of residual and Jacobian. This facilitates the use
of solver methods that evaluate the residual more frequantly than the Jacobian,
such as line search and trust region strategies.

```
auto computeResidual = [&](const NestedSolve::Value<> & guess,
                           NestedSolve::Value<> & residual) {
  residual(0) = guess(0) + guess(0) * guess(1) - 4;
  residual(1) = guess(0) + guess(1) - 3;
};

auto computeJacobian = [&](const NestedSolve::Value<> & guess,
                           NestedSolve::Jacobian<> & jacobian) {
  jacobian(0, 0) = 1 + guess(1);
  jacobian(0, 1) = guess(0);
  jacobian(1, 0) = 1;
  jacobian(1, 1) = 1;
};
```

We then use the three argument version of the `nonlinear` method

```
solver.nonlinear(solution, computeResidual, computeJacobian);
```

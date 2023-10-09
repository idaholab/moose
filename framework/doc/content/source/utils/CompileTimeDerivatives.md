# Compile-time Derivatives

`CompileTimeDerivatives` (CTD) is a C++ namespace containing classes, functions, and operators to implement mathematical expressions with the ability to perform symbolic automatic differentiation at compile time. It is a replacement for the runtime automatic differentiation in [`ExpressionBuilder`](/ExpressionBuilder.md optional=true) which uses a convoluted process to arrive at compiled mathematical expressions and their derivatives.

## Uses

The intended uses for the CTD framework are the implementation of empirical or analytical models with closed form expressions that compute quantities of which derivatives are required. Examples are

1. Thermodynamic free energies, the derivatives of which are chemical potentials, which are required to solve the phase field equations.
2. A thermal conductivity as a function of temperature, where the derivative w.r.t. temperature is required in the heat transfer equations.

CTD is not meant to replace runtime AD using dual numbers (which computes derivatives w.r.t. degrees of freedom). Any application that requires the construction of symbolic derivatives of equations w.r.t. known coupled variables, can be simplified with CTD.

## Examples

Constructing an expression and taking its derivative.

Create a _tag_ that will identify the variable

```C++
  enum
  {
    dX
  };
```

Connect a C++ variable and a _tag_ to obtain a compile time derivative reference object that will evaluate as the C++ variable value:

```C++
  Real x;
  const auto X = makeRef<dX>(x);
```

Build an expression for `x^2+100`:

```C++
  const auto result = X * X + 100.0;
```

Evaluate the expression for x=5:

```
  x = 5;
  Moose::out << result() << '\n'; // 125.0
```

And evaluate the derivative w.r.t. `x` at x=5:

```
  Moose::out << result.D<dX>()() << '\n'; // 10.0
```

Note that there is no runtime cost to calling `.D<dX>()`. The derivatives are taken at compile time.

The unit tests contain more examples on how to use the system.

!listing unit/src/CompileTimeDerivativesTest.C

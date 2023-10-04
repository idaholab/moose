# Compile-time Derivatives

`CompileTimeDerivatives` is a C++ namespace conating classes, functions, and operators to implement mathematical expressions with the ability to perform symbolic automatic differentiation at compile time. It is a replacement for the runtime automatic differentiation in [`ExpressionBuilder`](/ExpressionBuilder.md optional=true) which uses a convoluted process to arrive at compiled mathematical expressions and their derivatives.

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

And evaluate the derivative w.r.t. to `x` at x=5:

```
  Moose::out << result.D<dX>()() << '\n'; // 10.0
```

Note that there is no runtime cost to calling `.D<dX>()`. The derivatives are taken at compile time.

The unit tests contain more examples on how to use the system.

!listing unit/src/CompileTimeDerivativesTest.C

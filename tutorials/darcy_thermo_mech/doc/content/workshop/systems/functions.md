# Function System

A system for defining analytic expressions based on the spatial location ($x$, $y$, $z$) and
time, $t$.

!---

A `Function` object is created by inheriting from `Function` and overriding the virtual `value()`
(and optionally other methods as well) functions.

Functions can be accessed in most MOOSE objects by calling `getFunction("name")`,
where "name" matches a name from the input file.

!---

## Function Use

Many objects exist in MOOSE that utilize a function, such as:

- `FunctionDirichletBC`
- `FunctionNeumannBC`
- `FunctionIC`
- `BodyForce`

Each of these objects has a "function" parameter which is set in the input file, and controls which
`Function` object is used.

!---

## Parsed Function

A `ParsedFunction` allow functions to be defined by strings directly in the input file, e.g.:

!listing parsed/function.i block=Functions

It is possible to include other functions, as shown above, as well as scalar variables and
postprocessor values with the function definition.

!---

## Default Functions

Whenever a `Function` object is added via `addParam()`, a default can be provided.

Both constant values and parsed function strings can be used as the default.

```cpp
// Adding a Function with a default constant
params.addParam<FunctionName>("pressure_grad", "0.5", "The gradient of ...");

// Adding a Function with a default parsed function
params.addParam<FunctionName>("power_history", "t+100*sin(y)", "The power history of ...");
```

A `ParsedFunction` or `ConstantFunction` object is automatically constructed based on the default
value if a function name is not supplied in the input file.

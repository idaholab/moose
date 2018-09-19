<!-- MOOSE Documentation Stub: Remove this when content is added. -->

# Functions System

## Overview

- Function objects allow you to evaluate analytic expressions based on the spactial location $(x,y,z)$ and time $t$.
- You can create a custom `Function` object by inheriting from Function and overriding the virtual `value()` (and optionally `gradient()`) functions.
- Functions can be accessed in MOOSE objects (BCs, ICs, Materials, etc.) by calling `getFunction("name")`, where "name" matches a name from the input file.
- MOOSE has several built-in `Functions` such as:

  - `FunctionDirichletBC`
  - `FunctionNeumannBC`
  - `FunctionIC`
  - `UserForcingFunction`

- Each of these types has a "function" parameter which is set in the input file, and controls which `Function` object is used.
- `ParsedFunction` objects are defined by strings directly in the input file, e.g.:

  - `value = 'x*x+sin(y*t)'`

## Default Functions

- Whenever a `Function` object is added via `addParam()`, a default can be provided.
- Both constant values and parsed function strings can be used as default.

```cpp
   ... 
   // Adding a Function with a default constant
   params.addParam<FunctionName>("pressure_grad", "0.5", "doc");

   // Adding a Function with a default parsed function
   params.addParam<FunctionName>("power_history", "t+100*sin(y)", "doc");
   ...
```

- A `ParsedFunction` or `ConstantFunction` object is automatically constructed based on the default value if a function name is not supplied in the input file.

## Input File Syntax

- Functions are declared in the Function block.
- `ParsedFunction` allows you to provide a string specifying the function.
- You can use constants (like `alpha`), and define their value.
- Common expressions like `sin()` and `pi` are supported.
- After you have declared a `Function`, you can use them in objects like `FunctionDirichletBC`.

```puppet
... 
[Functions]
  active = 'bc_func'
  [./bc_func]
    type = ParsedFunction
    value = 'sin(alpha*pi*x)'
    vars = 'alpha'
    vals = '16'
  [../]
[]  
[BCs]
  active = 'all'
  [./all]
    type = FunctionDirichletBC
    variable = u 
    boundary = '1 2'
    function = bc_func
  [../]
[]
```

## Example 13: Custom Functions

In Example 13, two functions are created:

- A `ParsedFunction` that will be used to define a Dirichlet boundary condition
- A custom C++ `Function` object that defines a forcing function for the diffusion equation.

Look at [Example 13](ex13_functions.md)

## Further Function Documentation

!syntax list /Functions objects=True actions=False subsystems=False

!syntax list /Functions objects=False actions=False subsystems=True

!syntax list /Functions objects=False actions=True subsystems=False


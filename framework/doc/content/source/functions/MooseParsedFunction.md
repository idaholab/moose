# ParsedFunction

!syntax description /Functions/ParsedFunction

## Description

The `ParsedFunction` function takes a mathematical expression in [!param](/Functions/ParsedFunction/expression).  The
expression can be a function of time (t) or coordinate (x, y, or z).  The expression
can include common mathematical functions.  Examples include `4e4+1e2*t`,
`sqrt(x*x+y*y+z*z)`, and `if(t<=1.0, 0.1*t, (1.0+0.1)*cos(pi/2*(t-1.0)) - 1.0)`.

Additional variables may be declared in the [!param](/Functions/ParsedFunction/symbol_names) parameter vector. The
corresponding [!param](/Functions/ParsedFunction/symbol_values) parameter vector should list the items these variables are
bound to. Variables can be bound to:

- Constant number literals (for example `symbol_names = kB` and `symbol_values = 8.61733e-5`)
- A PostProcessor name (providing the value from the PP's last execution)
- A Function name (providing an immediate evaluation of the specified function)
- A scalar variable name


Further information can be found at the
[function parser site](http://warp.povusers.org/FunctionParser/).

!alert warning title=Scalar Variable Jacobian Contributions Omitted
Note that if this function is used for any residual contribution, inclusion of
scalar variables in the `expression` parameter will result in missing Jacobian
contributions, even if using [Automatic differentiation](automatic differentiation/index.md),
since `Function`s can currently only return `Real` values, not `ADReal` values.

## Example Input Syntax

!listing examples/ex13_functions/ex13.i block=Functions

!syntax parameters /Functions/ParsedFunction

!syntax inputs /Functions/ParsedFunction

!syntax children /Functions/ParsedFunction

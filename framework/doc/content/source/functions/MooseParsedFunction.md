# ParsedFunction

!syntax description /Functions/ParsedFunction

## Description

The `ParsedFunction` function takes a mathematical expression in `value`.  The
expression can be a function of time (t) or coordinate (x, y, or z).  The expression
can include common mathematical functions.  Examples include `4e4+1e2*t`,
`sqrt(x*x+y*y+z*z)`, and `if(t\textless=1.0, 0.1*t, (1.0+0.1)*cos(pi/2*(t-1.0)) - 1.0)`.

Additional variables may be declared in the `vars` parameter vector. The
corresponding `vals` parameter vector should list the items these variables are
bound to. Variables can be bound to:

- Constant number literals (for example `vars = kB` and `vals = 8.61733e-5`)
- A PostProcessor name (providing the value from the PP's last execution)
- A Function name (providing an immediate evaluation of the specified function)
- A scalar variable name

Further information can be found at the
[function parser site](http://warp.povusers.org/FunctionParser/).

## Example Input Syntax

!listing examples/ex04_bcs/trapezoid.i block=Functions

!syntax parameters /Functions/ParsedFunction

!syntax inputs /Functions/ParsedFunction

!syntax children /Functions/ParsedFunction

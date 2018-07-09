# ParsedFunction

!syntax description /Functions/ParsedFunction

## Description

The `ParsedFunction` function takes a mathematical expression in `value`.  The
expression can be a function of time (t) or coordinate (x, y, or z).  The expression
can include common mathematical functions.  Examples include `4e4+1e2*t`,
`sqrt(x*x+y*y+z*z)`, and `if(t\textless=1.0, 0.1*t, (1.0+0.1)*cos(pi/2*(t-1.0)) - 1.0)`.

Constant variables may be used in the expression if they have been declared with `vars`
and defined with `vals`.

Further information can be found at the
[function parser site](http://warp.povusers.org/FunctionParser/).

## Example Input Syntax

!listing examples/ex04_bcs/trapezoid.i block=Functions

!syntax parameters /Functions/ParsedFunction

!syntax inputs /Functions/ParsedFunction

!syntax children /Functions/ParsedFunction

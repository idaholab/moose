# PiecewiseConstant

!syntax description /Functions/PiecewiseConstant

## Description

The `PiecewiseConstant` function defines the data using a set of x-y data pairs.  Instead
of linearly interpolating between the values, however, the `PiecewiseConstant` function
is constant when the abscissa is between the values provided by the user.  The `direction`
parameter controls whether the function takes the value of the abscissa of the
user-provided point to the right or left of the value at which the function is evaluated.

## Example Input Syntax

!listing test/tests/functions/piecewise_constant/piecewise_constant.i block=Functions

!syntax parameters /Functions/PiecewiseConstant

!syntax inputs /Functions/PiecewiseConstant

!syntax children /Functions/PiecewiseConstant

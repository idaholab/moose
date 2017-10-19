# FunctionDT
!syntax description /Executioner/TimeStepper/FunctionDT

## Description
The `FunctionDT` type of TimeStepper takes time steps that vary over time
according to a user-defined function.

The time step is controlled by a piecewise linear function defined using the
`time_t` and `time_dt` parameters. A vector of time steps is provided using the
`time_dt` parameter. An accompanying vector of corresponding times is specified
using the `time_t` parameter. These two vectors are used to form a time step vs.
time function. The time step for a given step is computed by linearly
interpolating between the pairs of values provided in the vectors.

The same procedure that is used with
[ConstantDT](/Executioner/TimeStepper/framework/ConstantDT.md) is used to reduce
the time step from the user-specified value if a failed solution occurs.

## Example Input Syntax
!listing /test/tests/time_steppers/function_dt/function_dt_min.i block=Executioner

!syntax parameters /Executioner/TimeStepper/FunctionDT

!syntax inputs /Executioner/TimeStepper/FunctionDT

!syntax children /Executioner/TimeStepper/FunctionDT

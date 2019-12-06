# CoarsenedPiecewiseLinear

!syntax description /Functions/CoarsenedPiecewiseLinear

## Description

The `CoarsenedPiecewiseLinear` performs preprocessing and linear interpolation
on an x/y data set. The object  acts like
[`PiecewiseLinear`](/PiecewiseLinear.md)  except that it reduces the number of
function point at the start of the simulation. It uses the
[Ramer-Douglas-Peucker algorithm](https://en.wikipedia.org/wiki/Ramer%E2%80%93Douglas%E2%80%93Peucker_algorithm)
for data reduction.

## Example Input Syntax

!listing test/tests/misc/check_error/function_file_test1.i block=Functions

!syntax parameters /Functions/CoarsenedPiecewiseLinear

!syntax inputs /Functions/CoarsenedPiecewiseLinear

!syntax children /Functions/CoarsenedPiecewiseLinear

# PiecewiseLinear

!syntax description /Functions/PiecewiseLinear

## Description

The `PiecewiseLinear` function performs linear interpolations between user-provided
pairs of x-y data.  The x-y data can be provided in three ways. The first way is through
a combination of the `x` and `y` paramaters, which are lists of the x and y coordinates
of the data points that make up the function.  The second way is in the `xy_data`
parameter, which is a list of pairs of x-y data that make up the points of the
function.  This allows for the function data to be specified in columns by inserting line
breaks after each x-y data point.  Finally, the x-y data can be provided in an external
file containing comma-separated values.  The file name is provided in `data_file`,
and the data can be provided in either rows (default) or columns, as specified in the
`format` parameter.

By default, the x-data corresponds to time, but this can be changed to correspond to x, y,
or z coordinate with the `axis` line.  If the function is queried outside of its range of
x data, it returns the y value associated with the closest x data point, unless
the parameter `extrap` is set to `true`, in which case extrapolation is performed instead.

## Example Input Syntax

!listing test/tests/misc/check_error/function_file_test1.i block=Functions

!syntax parameters /Functions/PiecewiseLinear

!syntax inputs /Functions/PiecewiseLinear

!syntax children /Functions/PiecewiseLinear

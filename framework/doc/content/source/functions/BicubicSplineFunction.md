# BicubicSplineFunction

!syntax description /Functions/BicubicSplineFunction

The `BicubicSplineFunction` defines a 2D spline shape, which can be evaluated everywhere
in the domain by translation. The 2D plane for defining the spline is set by specifying the
`normal` parameter.

The spline is uniquely defined by:

- its values the 2D plane at the (`x1`, `x2`) points, given by the `y` parameter. The points
  form a 2D grid, which each `x1` being the abscissa for a line in this grid, with points
  at each `x2` specified

- its derivatives along `x1` and `x2` at the points on each extremity, given by `yx11`, `yx1n`, `yx21`, `yx2n`

- a functional form for the derivative along both directions, given by `yx1` and `yx2`

From this information the bicubic spline is automatically generated. Both the first and second
order derivatives of the spline are defined.

## Example input syntax

In this example, we define a bicubic spline from a list of points and derivatives. The `z` normal is
assumed by default and the bicubic spline is defined in the XY plane. The grid
for the points has 3 points along the `x` direction and 4 points along the `y` direction.

!listing test/tests/utils/spline_interpolation/bicubic_spline_interpolation.i block=Functions

!syntax parameters /Functions/BicubicSplineFunction

!syntax inputs /Functions/BicubicSplineFunction

!syntax children /Functions/BicubicSplineFunction

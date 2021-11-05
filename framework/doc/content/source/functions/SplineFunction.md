# SplineFunction

!syntax description /Functions/SplineFunction

The `SplineFunction` defines a 1D spline (1D shape, it is defined everywhere in the domain by translation)
along one of the x, y, z directions (specified by the `axis` parameter). The spline is defined by

- its value (ordinates, `y`) at several abscissa (`x`) through which the function passes, which can be
 given as a list of values in the input file, or given by a file reference

- its first derivative at the first abscissa

- its first derivative at the last abscissa


From this information the spline is automatically generated. Both the first and second
order derivatives of the spline are defined.

## Example input syntax

In this example, we define a spline going through 4 points defined by the `x` and `y`
parameters.

!listing test/tests/ics/function_ic/spline_function.i block=Functions

!syntax parameters /Functions/SplineFunction

!syntax inputs /Functions/SplineFunction

!syntax children /Functions/SplineFunction

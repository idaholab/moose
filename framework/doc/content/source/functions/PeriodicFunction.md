# PeriodicFunction

!syntax description /Functions/PeriodicFunction

## Description

The `PeriodicFunction` takes a user-supplied base function, and periodically
repeats that function in time or any of the three Cartesian coordinate
directions. The function can be repeated in any combination of these directions
or time.

The resulting function repeats the base function's
behavior over the interval from 0 to the specified period in a given dimension.
This is repeated indefinitely, and is also repeated (rather than mirrored)
for negative values of the coordinates or time.

## Example Input Syntax

!listing test/tests/functions/periodic_function/periodic_function.i block=Functions/periodic_xyzt

!syntax parameters /Functions/PeriodicFunction

!syntax inputs /Functions/PeriodicFunction

!syntax children /Functions/PeriodicFunction

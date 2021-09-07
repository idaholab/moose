>  Look through the input file and try to identify places where units might be
>  relevant.

There are four lines in the input file:

1. Two lines for the physical mesh dimension (`xmax` and `ymax`)
2. One lines for the Young's modulus
3. One line for the time step `dt`

Note that the Poisson's ratio is unitless.

So what are the chosen units here? It is up to the author of the input file to
decide that, as long as the units of the supplied parameters are consistent.

Let's start with the Young's modulus and say its units are Pa. Pascal is an SI
derived unit for pressure and is equivalent to
$\frac{N}{m^2} = \frac{kg}{m\cdot s^2}$.
From that the consistent time and length units follow immediately.  The
mesh dimensions `1 x 2` must have the unit meters, the time must be specified in
seconds.

!alert note
MOOSE provides a system to convert quanities between units right in the input
file. A list of supported units can be found [here](utils/Units.md). Try using
`youngs_modulus = ${units 145037.74 lbs/in^2 -> Pa}` in the input file, for an
explicit conversion from pounds per square inch to Pascal.

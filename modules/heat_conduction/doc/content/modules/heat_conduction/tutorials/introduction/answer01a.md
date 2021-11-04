>  Look through the input file and try to identify places where units might be
>  relevant.

There are four locations in the input file:

1. Two lines for the physical mesh dimension (`xmax` and `ymax`)
2. A block defining the temperature variable
3. One line defining the thermal conductivity
4. One line defining the time step `dt`

So what are the chosen units here? It is up to the author of the input file to
decide that, as long as the units of the supplied parameters are consistent.

Let's start with the thermal conductivity and say its units are in Watts per meter-Kelvin
($W/(m\cdot K)$). To be consistent with this, the mesh dimensions must be in meters,
and the temperature should be in Kelvin. Note that the temperature could also be
defined in Celsius, because the temperature increments are the same for these
two units. The choice of Celsius or Kelvin would be defined by boundary conditions
or temperature-dependent properties, which would depend on a specific unit. The
time unit should be in seconds to be consistent with the SI base units that the
Watt is based on.

!alert note
MOOSE provides a system to convert quanities between units right in the input
file. A list of supported units can be found [here](utils/Units.md).

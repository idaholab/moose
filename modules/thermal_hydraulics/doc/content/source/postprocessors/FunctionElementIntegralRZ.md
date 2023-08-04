# FunctionElementIntegralRZ

!syntax description /Postprocessors/FunctionElementIntegralRZ

The integral is computed as:

!equation
I = \int_\Omega f(t, \vec{M}) r(\vec{M}) d\Omega

where $I$ is the integral, $f$ is the function evaluated at the current time $t$ and position $\vec{M}$,
and $r$ is the local circumference.
This volumetric integral is computed numerically using the system's quadrature.

!alert warning
With the recent development of general RZ coordinates, this object along with all THM's "RZ"-specific
objects will soon be deprecated in favor of more general 2D objects used in general RZ coordinates.
Stay tuned!

!syntax parameters /Postprocessors/FunctionElementIntegralRZ

!syntax inputs /Postprocessors/FunctionElementIntegralRZ

!syntax children /Postprocessors/FunctionElementIntegralRZ

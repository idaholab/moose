# ADSideFluxIntegralRZ

!syntax description /Postprocessors/ADSideFluxIntegralRZ

The diffusive flux $I$ over the cylindrical surface is computed as:

!equation
I = \int_{\partial \Omega} \nabla v \cdot \vec{n} r(\vec{M}) d\partial \Omega

where $v$ is the variable of interest, $\vec{n}$ the normal to the surface and $r(\vec{M})$ is
the local circumference.

!alert warning
With the recent development of general RZ coordinates, this object along with all THM's "RZ"-specific
objects will soon be deprecated in favor of more general 2D objects used in general RZ coordinates.
Stay tuned!

!syntax parameters /Postprocessors/ADSideFluxIntegralRZ

!syntax inputs /Postprocessors/ADSideFluxIntegralRZ

!syntax children /Postprocessors/ADSideFluxIntegralRZ

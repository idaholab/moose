# INSFVVelocityHydraulicSeparatorBC

This object serves three purposes:

- It prevents momentum flow through this face. This means that the
  advective flux is:

  $\rho u_i \vec{u}\cdot\vec{n} = 0~,$

  where $u_i$ is the i-th component of the velocity vector $\vec{u}$ on the face,
  while $\rho$ is the density on the face. While the normal component of
  the stress term is also zero.
- It disables this face's contribution to the Rhie-Chow interpolation.
- It forces a two-sided evaluation on this face during gradient computation
  using the Green-Gauss approach. This means that this face is a discontinuity,
  the velocity has different values from the element and the neighbor side on the face.

## Example input syntax

This example describes a 2D domain with an inlet and outlet mixing regions with a domain
separated into 3 parallel channels between them using two sidesets.

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/hydraulic-separators/separator-energy.i block=FVBCs

!syntax parameters /FVBCs/INSFVVelocityHydraulicSeparatorBC

!syntax inputs /FVBCs/INSFVVelocityHydraulicSeparatorBC

!syntax children /FVBCs/INSFVVelocityHydraulicSeparatorBC

# INSFVScalarFieldSeparatorBC

This object serves two purposes:

- It prevents scalar flux through this face. This means that the
  advective flux is:

  $\Phi \vec{u}\cdot\vec{n} = 0~,$

  where $\Phi$ is the scalar variable, $\vec{u}$ denotes the velocity vector on the face.
  Furthermore, this separator BC also ensures that the diffusive flux is zero on this boundary.
- It forces a two-sided evaluation on this face during gradient computation
  using the Green-Gauss approach. This means that this face is a discontinuity,
  the scalar field has different values from the element and the neighbor side on the face.

## Example input syntax

This example describes a 2D domain with an inlet and outlet mixing regions with a domain
separated into 3 parallel channels between them using two sidesets.

!listing modules/navier_stokes/test/tests/finite_volume/pins/channel-flow/hydraulic-separators/separator-energy.i block=FVBCs

!syntax parameters /FVBCs/INSFVScalarFieldSeparatorBC

!syntax inputs /FVBCs/INSFVScalarFieldSeparatorBC

!syntax children /FVBCs/INSFVScalarFieldSeparatorBC

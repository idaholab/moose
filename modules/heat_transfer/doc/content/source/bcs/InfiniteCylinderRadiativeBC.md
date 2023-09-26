# InfiniteCylinderRadiativeBC

!syntax description /BCs/InfiniteCylinderRadiativeBC

This boundary condition computes the radiative heat flux from the boundary to
an infinite cylinder completely surrounding it. The boundary itself is assumed
to be a cylinder that is concentric to the enclosing cylinder. Geometry information
is provided by boundary_radius and cylinder_radius parameters. The mesh is not queried for
geometric information.

!listing radiative_bcs/radiative_bc_cyl.i block=BCs/radiative_bc


!syntax parameters /BCs/InfiniteCylinderRadiativeBC

!syntax inputs /BCs/InfiniteCylinderRadiativeBC

!syntax children /BCs/InfiniteCylinderRadiativeBC

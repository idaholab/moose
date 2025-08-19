# PeriodicRayBC

!syntax description /RayBCs/PeriodicRayBC

The definition of the periodic boundaries is handled the same as in the standard
MOOSE periodic boundary condition system. This includes defining periodic boundaries:

- Automatically
- With a user-defined translation
- With a function transofmration

See the [periodic boundary condition syntax documentation](syntax/BCs/Periodic/index.md)
for more information.

!alert warning
Rays can be propagated across multiple periodic boundaries at the same time when BCs are
applied at points where one periodic boundary neighbors another (or multiple). However,
this capability is not fully supported with distributed meshes due to insufficient
element ghosting. A warning will be emitted if neighboring periodic boundaries are found
when using a distributed mesh.

## Example syntax

In this example, a 2D generated mesh is used and periodic ray boundaries are added
automatically in the x and y directions:

!listing periodic_ray_bc_2d.i block=RayBCs

!syntax parameters /RayBCs/PeriodicRayBC

!syntax inputs /RayBCs/PeriodicRayBC

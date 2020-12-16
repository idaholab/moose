# Ray Tracing Module

The ray tracing module traces rays through the finite element mesh. Notable features include:

- Supports tracing in meshes with planar sides (2D and 3D)
- Supports mesh adaptivity
- Supports contribution to residuals and Jacobians (with full coupling support) along ray segments
- Supports ray interaction with internal and external boundaries
- Supports storage and manipulation of data unique to each ray
- Supports ray interaction with field variables
- Highly parallelizable: tested to 20k MPI ranks

!row!
!col small=12 medium=6 large=6
!media large_media/ray_tracing/cone_ray_study_u.png
       id=cone-ray-study-u
       caption=Example of [flashlight_source.md] within a diffusion-reaction problem.

!col small=12 medium=6 large=6
!media large_media/ray_tracing/cone_ray_study_rays.png
      id=cone-ray-study-rays
      caption=Overlay of the rays that were traced for the problem in [cone-ray-study-u].
!row-end!

## Examples

- [line_integrals.md] - Integration of a field along a line
- [line_sources.md] - Body force term along a line
- [flashlight_source.md] - Anisotropic point sources that emit in a cone of directions

## Object Overview

- [Ray.md] - Basic data structure that represents a single ray that traverses the mesh
- [RayTracingStudy.md] - [UserObject](UserObjects/index.md) that generates and executes the [Rays](Ray.md)
- [RayBCs/index.md] - Manipulates [Rays](Ray.md) on boundaries, both internal and external
- [RayKernels/index.md] - Manipulates [Rays](Ray.md) on segments within an element along a [Ray.md] trajectory

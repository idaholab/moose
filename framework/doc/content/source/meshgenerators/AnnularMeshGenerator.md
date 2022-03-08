# AnnularMeshGenerator

!syntax description /Mesh/AnnularMeshGenerator

## Overview

This MeshGenerator object creates a mesh with an annular shape, and Quad4 elements distributed on several rings. The user can choose the inner and outer radii, as well as the number of elements in the radial and angular directions.

It is also possible to create a disc-shaped mesh with this class. To do so, the user has to choose 0 for the value of the inner radius. This will create a mesh composed of TRI3 elements at the center of the disc, and QUAD4 everywhere else.

If $dmin \neq 0$ and $dmax \neq 360$, this will create a fraction of an annulus or disc.

## Description

The AnnularMesh mesh generator builds simple 2D annular and disc meshes. They are created by drawing radial lines and concentric circles, and the mesh consists of the quadrilaterals thus formed. Therefore, no sophisticated paving is used to construct the mesh.

The inner radius and the outer radius must be specified. If the inner radius is zero a disc mesh is created, while if it is positive an annulus is created. The annulus has just one subdomain (block number = 0), whereas the disc has two subdomains: subdomain zero consists of the outer quadrilaterals, while the other (block number = 1) consists of the triangular elements that emanate from the origin.

The minimum and maximum angle may also be specified. These default to zero and 360, respectively. If other values are chosen, a sector of an annulus, or a sector of a disc will be created. Both angles are measured anti-clockwise from the xx axis.

The number of elements in the radial direction and the angular direction may be specified. By default, the mesh spacing is uniform in the radial direction, but the following options are available for more control over the radial meshing:
- A growth factor that controls the element size in the radial direction may be specified.  In this case, the radial element size in the innermost ring is multiplied by this factor for each concentric ring of elements, moving from the inner to the outer radius. If the growth factor is positive, the element radial dimension increases with increasing radial position, while if the growth factor is negative, the element radial dimension decreases with increasing radial position.
- A list of values that define the radial positions of the rings of interior nodes can be specified. This allows for direct control of the element size for every one of the concentric rings. In this case, the number of radial elements is not specified, because it is inferred from the length of that list.
- A flag that enforces equal areas among all of the elements. When true, this parameter will automatically determine the radial element spacing so that each element has the same area.

Sidesets are also created:

- Sideset 0 is called "rmin" and is the set of sides at the minimum radius (which is zero for the disc).
- Sideset 1 is called "rmax" and is the set of sides at the maximum radius.
- Sideset 2 is called "dmin" and is the set of sides at the minimum angle, which is created only in the case of a sector of an annulus (or disc)
- Sideset 3 is called "dmax" and is the set of sides at the maximum angle, which is created only in the case of a sector of an annulus (or disc)

!syntax parameters /Mesh/AnnularMeshGenerator

!syntax inputs /Mesh/AnnularMeshGenerator

!syntax children /Mesh/AnnularMeshGenerator

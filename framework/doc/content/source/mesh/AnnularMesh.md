# AnnularMesh

!syntax description /Mesh/AnnularMesh

## Description

The `AnnularMesh` mesh generator builds simple 2D annular and disc meshes.  They are created by drawing radial lines and concentric circles, and the mesh consists of the quadrilaterals thus formed.  Therefore, no sophisticated paving is used to construct the mesh.

The inner radius and the outer radius must be specified.  If the inner radius is zero a disc mesh is created, while if it is positive an annulus is created.  The annulus has just one subdomain (block number = 0), whereas the disc has two subdomains: subdomain zero consists of the outer quadrilaterals, while the other (block number = 1) consists of the triangular elements that emanate from the origin.

The minimum and maximum angle may also be specified.  These default to zero and 360, respectively.  If other values are chosen, a sector of an annulus, or a sector of a disc will be created.  Both angles are measured anti-clockwise from the $x$ axis.

The number of elements in the radial direction and the angular direction may be specified.  In addition, a growth factor on the element size in the radial direction may be chosen.  The element-size (in the radial direction) is multiplied by this factor for each concentric ring of elements, moving from the inner to the outer radius.

Sidesets are also created:

- Sideset 0 is called "rmin" and is the set of sides at the minimum radius (which is zero for the disc).
- Sideset 1 is called "rmax" and is the set of sides at the maximum radius.
- Sideset 2 is called "dmin" and is the set of sides at the minimum angle, which is created only in the case of a sector of an annulus (or disc)
- Sideset 3 is called "dmax" and is the set of sides at the maximum angle, which is created only in the case of a sector of an annulus (or disc)


## Example Syntax

A full annulus with minimum radius 1 and maximum radius 5, with smaller elements near the inside of the annulus.  (A disc would be created by setting rmin to zero.)

!listing test/tests/mesh/mesh_generation/annulus.i block=Mesh

A sector of an annulus, sitting between 45 and 135 degrees.  (A sector of a disc would be created by setting rmin to zero.)

!listing test/tests/mesh/mesh_generation/annulus_sector.i block=Mesh

An example of using sidesets

!listing test/tests/mesh/mesh_generation/annulus_sector.i block=BCs

!syntax parameters /Mesh/AnnularMesh

!syntax inputs /Mesh/AnnularMesh

!syntax children /Mesh/AnnularMesh

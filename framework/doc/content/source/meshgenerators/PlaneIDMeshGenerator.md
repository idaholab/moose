# PlaneIDMeshGenerator

!syntax description /Mesh/PlaneIDMeshGenerator

## Overview

The `PlaneIDMeshGenerator` object applies an extra IDs to axial planes defined in a extruded mesh.

## Extra ID Control

The `PlaneIDMeshGenerator` takes a mesh object and its axial layer structure as input.
This axial plane structure given in [!param](/Mesh/PlaneIDMeshGenerator/plane_coordinates) contains a list of coordinates defining each plane from bottom to top.
If there are N planes, N+1 coordinate points should be defined here.

If each axial plane is uniformly sub-divided into mutiple layers during the extrusion, distinct extra IDs can be optionally assigned to individual layers in each plane.
[!param](/Mesh/PlaneIDMeshGenerator/num_ids_per_plane) defines the number of unique IDs in each plane defined in [!param](/Mesh/PlaneIDMeshGenerator/plane_coordinates).

Note that this generator only works for extruded geometries where the concept of axial layer is valid.
The axis of plane can be specfied using [!param](/Mesh/PlaneIDMeshGenerator/plane_axis).

## Example Syntax

!listing test/tests/meshgenerators/plane_id_mesh_generator/plane_id_pin3d.i block=Mesh

!syntax parameters /Mesh/PlaneIDMeshGenerator

!syntax inputs /Mesh/PlaneIDMeshGenerator

!syntax children /Mesh/PlaneIDMeshGenerator

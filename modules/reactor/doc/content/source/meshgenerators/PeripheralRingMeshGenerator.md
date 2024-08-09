# PeripheralRingMeshGenerator

!syntax description /Mesh/PeripheralRingMeshGenerator

## Overview

The `PeripheralRingMeshGenerator` object adds a 2D peripheral region using either first-order or second-order quadrilateral elements (i.e., QUAD4 or QUAD9) with a circular external boundary to the input 2D mesh. The order of the elements is automatically determined by the order of the input mesh elements on its external boundary. Mixed element order is NOT supported. A common use of this mesh generator is to mesh the circular peripheral region of a reactor core. This mesh generator can be used recursively on a mesh to add as many rings of peripheral regions as desired.

## More Information

The use of this mesh generator requires an input mesh defined by [!param](/Mesh/PeripheralRingMeshGenerator/input) and the name/ID of its external boundary defined by [!param](/Mesh/PeripheralRingMeshGenerator/input_mesh_external_boundary). `PeripheralRingMeshGenerator` examines the external boundary of the input mesh to confirm whether the algorithm will function properly for the given external boundary. Trivially, the external boundary of the input mesh must be a single-segment closed loop.

This object then calculates the azimuthal angles of all the nodes on the external boundary of the input mesh. Each node on the input mesh external boundary is then used as the start point of a line segment, whose end point (node) lies on the circle defined by [!param](/Mesh/PeripheralRingMeshGenerator/peripheral_ring_radius). The segment is therefore aligned along the same azimuthal angle as the start point. Intermediate nodes may also be added if [!param](/Mesh/PeripheralRingMeshGenerator/peripheral_layer_num) is greater than unity. Currently, the algorithm only works when the azimuthal angles of the boundary nodes change monotonically as the external boundary is traversed. This object always generates a peripheral ring with its center at the origin, no matter where the centroid of the [!param](/Mesh/PeripheralRingMeshGenerator/input) mesh is. Optionally, [!param](/Mesh/PeripheralRingMeshGenerator/force_input_centroid_as_center) can be set as `true` to move the centroid of the [!param](/Mesh/PeripheralRingMeshGenerator/input) mesh to the origin.

The added peripheral block must be assigned an ID given by [!param](/Mesh/PeripheralRingMeshGenerator/peripheral_ring_block_id) along with an optional name given by [!param](/Mesh/PeripheralRingMeshGenerator/peripheral_ring_block_name). The new external boundary of the mesh can also be assigned an ID and a name through [!param](/Mesh/PeripheralRingMeshGenerator/external_boundary_id) and [!param](/Mesh/PeripheralRingMeshGenerator/external_boundary_name), respectively.

Optionally, users can preserve the volume of the peripheral ring region by setting [!param](/Mesh/PeripheralRingMeshGenerator/preserve_volumes) as `true` to enable correction of polygonization effect.

!media reactor/meshgenerators/core_ring.png
      style=display: block;margin-left:auto;margin-right:auto;width:40%;
      id=pattern_hex
      caption=The peripheral block (in teal) created by this mesh generator using the input shown in Example Syntax section.

In addition, the radial meshing density of the peripheral ring region can be biased by setting the [!param](/Mesh/PeripheralRingMeshGenerator/peripheral_radial_bias) parameter. The bias value should be a positive `Real` type parameter, which is the radial dimension ratio between two radially-neighboring elements (outer to inner).

Aside from the general mesh radial biasing options described above, users can also define boundary layers within the peripheral ring region. Both inner and outer boundary layers are supported. Each boundary layer has three key parameters that need to be provided:

- [!param](/Mesh/PeripheralRingMeshGenerator/peripheral_inner_boundary_layer_width)/[!param](/Mesh/PeripheralRingMeshGenerator/peripheral_outer_boundary_layer_width): the radiation width of the boundary layer within the region. Note that the summation of the inner and outer boundary layers width must be smaller than the thinnest part of the peripheral ring.
- [!param](/Mesh/PeripheralRingMeshGenerator/peripheral_inner_boundary_layer_intervals)/[!param](/Mesh/PeripheralRingMeshGenerator/peripheral_outer_boundary_layer_intervals): the number of radial mesh discretization of the boundary layer.
- [!param](/Mesh/PeripheralRingMeshGenerator/peripheral_inner_boundary_layer_bias)/[!param](/Mesh/PeripheralRingMeshGenerator/peripheral_outer_boundary_layer_bias): the growth factor used for radial mesh biasing for the boundary layer.

!media reactor/meshgenerators/peripheral_boundary_layer.png
      style=display: block;margin-left:auto;margin-right:auto;width:40%;
      id=boundary_layer
      caption=The peripheral block (in teal) created by this mesh generator with biased boundary layers.

## Example Syntax

!listing modules/reactor/test/tests/meshgenerators/peripheral_ring_mesh_generator/core_ring.i block=Mesh/pr

!syntax parameters /Mesh/PeripheralRingMeshGenerator

!syntax inputs /Mesh/PeripheralRingMeshGenerator

!syntax children /Mesh/PeripheralRingMeshGenerator

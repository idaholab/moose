# PeripheralRingMeshGenerator

!syntax description /Mesh/PeripheralRingMeshGenerator

## Overview

The `PeripheralRingMeshGenerator` object adds a peripheral region using first-order quadrilateral elements (i.e., QUAD4) with a circular external boundary to the input mesh. A common use of this mesh generator is to mesh the circular peripheral region of a reactor core. This mesh generator can be used recursively on a mesh to add as many rings of peripheral regions as desired.

## More Information

The use of this mesh generator requires an input mesh defined by [!param](/Mesh/PeripheralRingMeshGenerator/input) and the name/ID of its external boundary defined by [!param](/Mesh/PeripheralRingMeshGenerator/input_mesh_external_boundary). `PeripheralRingMeshGenerator` examines the external boundary of the input mesh to confirm whether the algorithm will function properly for the given external boundary. Trivially, the external boundary of the input mesh must be a single-segment closed loop.

This object then calculates the azimuthal angles of all the nodes on the external boundary of the input mesh. Each node on the input mesh external boundary is then used as the start point of a line segment, whose end point (node) lies on the circle defined by [!param](/Mesh/PeripheralRingMeshGenerator/peripheral_ring_radius). The segment is therefore aligned along the same azimuthal angle as the start point. Intermediate nodes may also be added if [!param](/Mesh/PeripheralRingMeshGenerator/peripheral_layer_num) is greater than unity. Currently, the algorithm only works when the azimuthal angles of the boundary nodes change monotonically as the external boundary is traversed.

The added peripheral block must be assigned an ID given by [!param](/Mesh/PeripheralRingMeshGenerator/peripheral_ring_block_id) along with an optional name given by [!param](/Mesh/PeripheralRingMeshGenerator/peripheral_ring_block_name). The new external boundary of the mesh can also be assigned an ID and a name through [!param](/Mesh/PeripheralRingMeshGenerator/external_boundary_id) and [!param](/Mesh/PeripheralRingMeshGenerator/external_boundary_name), respectively.

Optionally, users can preserve the volume of the peripheral ring region by setting [!param](/Mesh/PeripheralRingMeshGenerator/preserve_volumes) as `true` to enable correction of polygonization effect.

!media reactor/meshgenerators/core_ring.png
      style=display: block;margin-left:auto;margin-right:auto;width:40%;
      id=pattern_hex
      caption=The peripheral block (in teal) created by this mesh generator using the input shown in Example Syntax section.

## Example Syntax

!listing modules/reactor/test/tests/meshgenerators/peripheral_ring_mesh_generator/core_ring.i block=Mesh/pr

!syntax parameters /Mesh/PeripheralRingMeshGenerator

!syntax inputs /Mesh/PeripheralRingMeshGenerator

!syntax children /Mesh/PeripheralRingMeshGenerator

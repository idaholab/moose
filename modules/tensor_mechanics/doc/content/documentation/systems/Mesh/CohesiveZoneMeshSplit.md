# Cohesive Zone Mesh Split

!syntax description /Mesh/CohesiveZoneMeshSplit

This class implement a methods to split a monolithic mesh by blocks and is intended to be used with the Interface Kernel Cohesive Zone Model.

To split the mesh nodes belonging to a boundary between 2 blocks are duplicated, the mesh is split and a new sideset is added. The new sideset is the boundary where the InterfaceKernel lives.

As an option, the interface can be split into $N$ different sidesets. $N$ is the number of adjacent block pairs. This is achieved by setting  `split_interface=true`. This is useful when modeling interfaces with different cohesive parameters or different cohesive laws.

## Example Input File Syntax



### Single interface

!listing modules/tensor_mechanics/test/tests/cohesive_zone_IK/2D/czmTestSplitMesh_2DJunction_Auto.i block=Mesh

### Multiple interfaces

When `split_interface=true` the cohesive interface is split by block pairs:

!listing modules/tensor_mechanics/test/tests/cohesive_zone_IK/2D/czmTestSplitMesh_2DJunction_splitTrue_Auto.i block=Mesh

The naming convention for the interface is `czm_bMi_bSj`, where $i$ and $j$ represents the original blocks ids.
This convention also assumes that the master interface $i$ is set on the block with lower id, while the slave interface $j$ is always on the block with higher id.
Note that the new generated cohesive interfaces might not have consecutive ids, therefore interfaces should be referred by name like in the following example:

!listing modules/tensor_mechanics/test/tests/cohesive_zone_IK/2D/czmTest3DC_CohesiveLaw2D_splitInterface.i block=InterfaceKernel

!syntax inputs /Mesh/CohesiveZoneMeshSplit

!syntax parameters /Mesh/CohesiveZoneMeshSplit

# BreakMeshByBlockGenerator

!syntax description /Mesh/BreakMeshByBlockGenerator

## Description

This class implements a MeshGenerator to split a monolithic mesh by blocks similar to what is proposed by VP Nguyen [!cite](Nguyen2014).

To split the mesh, nodes shared by multiple blocks are duplicated N-1 times (where N is the number of blocks sharing a particular node). Each duplicated node is assigned to one block and all the elements sharing that node are updated. Separate directional sidesets are always created on both sides of each cut. Their names are formed from the adjacent block names, and each direction has its own boundary ID. These boundary pairs allow libMesh to reconstruct disjoint neighbors on distributed meshes.

Two block restriction options are provided: [!param](/Mesh/BreakMeshByBlockGenerator/surrounding_blocks) and [!param](/Mesh/BreakMeshByBlockGenerator/block_pairs). The [!param](/Mesh/BreakMeshByBlockGenerator/surrounding_blocks) option creates interfaces both between the listed blocks and between the listed blocks and the rest of the mesh. The [!param](/Mesh/BreakMeshByBlockGenerator/block_pairs) option creates interfaces only between the specified block pairs. Multiple block pairs are separated by semicolons (`;`). These two options cannot be used together. When neither is specified, interfaces are generated between all adjacent blocks.

## Example Input File Syntax

### Mesh Splitting

!listing test/tests/meshgenerators/break_mesh_by_block_generator/break_mesh_2DJunction_auto.i block=Mesh

### Interface Naming

Each interface is named by joining the block names in the order of the side's
block followed by its neighbor's block. For neighboring blocks named `wood`
and `steel`, `wood_steel` contains sides on `wood` and `steel_wood` contains
sides on `steel`. If a block is unnamed, its name defaults to `Block` plus its
block ID; for example, `wood_Block2` and `Block2_wood`.

Specify the desired directional block-pair names in the `boundary` parameter of
interface kernels and cohesive zone models. For example, `boundary = wood_steel`
selects the sides on `wood` facing `steel`. An interface kernel integrates its
weak form on these selected sides, treating the elements in `wood` as the element
side and those in `steel` as the neighbor side. Selecting `steel_wood` reverses
these roles.

### block_pairs option

The [!param](/Mesh/BreakMeshByBlockGenerator/block_pairs) option restricts splitting to the specified block pairs. Nodes shared by more than two blocks are left connected at junctions. In the example below, three different cases are shown, where one, two and three new nodes are created, respectively.

!media media/meshgenerators/3_blocks_example.png style=width:100%;
       alt=Illustration of the effect of using different values for the block_pairs parameter.

!listing test/tests/meshgenerators/break_mesh_by_block_generator/break_mesh_block_pairs_restricted_3blocks.i
         block=Mesh

!syntax parameters /Mesh/BreakMeshByBlockGenerator

!syntax inputs /Mesh/BreakMeshByBlockGenerator

!syntax children /Mesh/BreakMeshByBlockGenerator

!bibtex bibliography

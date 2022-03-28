# BreakMeshByBlockGenerator

!syntax description /Mesh/BreakMeshByBlockGenerator

## Description

This class implements a MeshGenerator to split a monolithic mesh by blocks similar to what is proposed by VP Nguyen [!cite](Nguyen2014).

To split the mesh, nodes shared by multiple blocks are duplicated N-1 times (where N is the number of blocks sharing a particular node). Each duplicated nodes is assigned to one block and all the elements sharing that node are updated. A new sideset identifying the new interface is added, and it is always linked to elements belonging to blocks with the lower ID.

As an option, the interface can be split into $Q$ different sidesets, where $Q$ is the number of adjacent block pairs. This is achieved by setting `split_interface=true` and is useful when modeling interfaces with different parameters.

Two types of block restricted option are provided, namely `surrounding_blocks` and `block_pairs`. The `surrounding_blocks` will create interfaces surrounding specified block. When `surrounding_blocks` is provided, the additional boundary `interface_transition` can be added by setting `add_transition_interface=true`. The `interface_transition` boundary identifies the interface between the provided blocks and the rest of the mesh. The `block_pairs` option will create interfaces  only \emph{between} the specified block pairs. Multiple block pairs can be provided, and they are separated by semicolon (`;`). The `block_pairs` option does not work with `add_transition_interface`. By default when no block restricted option is used, interfaces will be generated between all blocks. In addition, the `add_interface_on_two_sides` parameter allows to generate interface boundaries on both sides of the interface which can be used to enforce thermal and mechanical contact at the interface.

## Example Input File Syntax

### Single Interface

!listing test/tests/meshgenerators/break_mesh_by_block_generator/break_mesh_2DJunction_auto.i block=Mesh

### Multiple Interfaces

When `split_interface=true`, the new generated interface is split by block pairs
and named by joining the block names. For instance if one has two neighboring
blocks - one named `wood` and named `steel` with `blockID` equal to `1` and `2`,
respectively - the new interface will be named `wood_steel`. The naming order
follows the block ID order. For this simple example, the new sideset will be on
the block named `wood`. If one block is not named, its name will default to
`Block` plus the `blockID`. For instance, if block 2 is not named the new
interface will be named `wood_Block2`.

!listing test/tests/meshgenerators/break_mesh_by_block_generator/break_mesh_2DJunction_splittrue.i block=Mesh

### block_pair option

For `block_pair` option, only the nodes that are shared by specific block pairs will be newly created. In the example below, three different cases are shown, where one, two and three new nodes are created, respectively.

!media media/meshgenerators/3_blocks_example.png style=width:100%;

!listing test/tests/meshgenerators/break_mesh_by_block_generator/break_mesh_block_pairs_restricted_3blocks.i
         block=Mesh

!syntax parameters /Mesh/BreakMeshByBlockGenerator

!syntax inputs /Mesh/BreakMeshByBlockGenerator

!syntax children /Mesh/BreakMeshByBlockGenerator

!bibtex bibliography

# BreakMeshByBlockGenerator

!syntax description /Mesh/BreakMeshByBlockGenerator

## Description

This class implements a MeshGenerator to split a monolithic mesh by blocks similar to what is proposed by VP Nguyen [!cite](Nguyen2014).

To split the mesh, nodes shared by multiple blocks are duplicated N-1 times (where N is the number of blocks sharing a particular node). Each duplicated node is assigned to one block and all the elements sharing that node are updated. Separate directional sidesets are always created on both sides of each cut. Their names are formed from the adjacent block names, and each direction has its own boundary ID. These boundary pairs allow libMesh to reconstruct disjoint neighbors on distributed meshes.

Two block restriction options are provided: `surrounding_blocks` and `block_pairs`. The `surrounding_blocks` option creates interfaces both between the listed blocks and between the listed blocks and the rest of the mesh. The `block_pairs` option creates interfaces only between the specified block pairs. Multiple block pairs are separated by semicolons (`;`). These two options cannot be used together. When neither is specified, interfaces are generated between all adjacent blocks.

The parameters `split_interface`, `add_interface_on_two_sides`, and `split_transition_interface` are deprecated. Remove them from input files. Explicitly setting any of them to `true` produces a deprecation warning; setting one to `false` produces an error. Their behavior is now unconditional, including for interfaces between `surrounding_blocks` and the rest of the mesh. The parameters `interface_name`, `add_transition_interface`, and `interface_transition_name` have been removed. Replace references to the former combined `interface` or `interface_transition` boundaries with the appropriate directional block-pair names.

Elements on the two sides of a generated interface are connected as disjoint neighbors, which libMesh matches by the position of their sides. Because the two sides separate when the mesh deforms, a broken mesh cannot be used with a displaced mesh: set `use_displaced_mesh = false` in the `[Mesh]` block. Finite deformation cohesive zone models can use the Lagrangian solid mechanics kernels (`new_system = true`), which work on the reference mesh, or the mortar-based cohesive zone model, which does not rely on pairing elements through the disjoint neighbor machinery.

## Example Input File Syntax

### Mesh Splitting

!listing test/tests/meshgenerators/break_mesh_by_block_generator/break_mesh_2DJunction_auto.i block=Mesh

### Interface Naming

Each interface is named by joining the block names in the order of the side's
block followed by its neighbor's block. For neighboring blocks named `wood`
and `steel`, `wood_steel` contains sides on `wood` and `steel_wood` contains
sides on `steel`. If a block is unnamed, its name defaults to `Block` plus its
block ID; for example, `wood_Block2` and `Block2_wood`.

A combined `interface` boundary can contain several block pairs, so applying a
cohesive law to it may also affect interfaces the user did not intend to include.
Explicit block-pair names make the selected interfaces clear in the input file.

### block_pair option

For `block_pair` option, only the nodes that are shared by specific block pairs will be newly created. In the example below, three different cases are shown, where one, two and three new nodes are created, respectively.

!media media/meshgenerators/3_blocks_example.png style=width:100%;
       alt=Illustration of the effect of using different values for the block_pairs parameter.

!listing test/tests/meshgenerators/break_mesh_by_block_generator/break_mesh_block_pairs_restricted_3blocks.i
         block=Mesh

!syntax parameters /Mesh/BreakMeshByBlockGenerator

!syntax inputs /Mesh/BreakMeshByBlockGenerator

!syntax children /Mesh/BreakMeshByBlockGenerator

!bibtex bibliography

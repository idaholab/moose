# MooseMesh

MooseMesh is the base class that all mesh objects in MOOSE must inherit from. MooseMesh is responsible for holding the underlying
data structures holding the actual mesh (through libMesh) and caches for many commonly accessed entities. MOOSE currently requires
an active Mesh for every simulation even if the mesh is not strictly necessary when wrapping a third party application or running
non-FE type calculations. The Mesh however can consist of just a single element.

## Sidesets and Nodesets

MOOSE can apply several different objects to side sets and node sets to either compute quantities of interest or set boundary
conditions. It is important to understand the different between these two concepts. Nodesets are simply a set of nodes, typically on
a boundary, that all contain a common ID (the node set ID). These can be created and assigned in your mesh builder program (such
as Cubit or GMsh). Sidesets are a collectio of $dim - 1$ elements (sides or edges), typically on a boundary or on a plane within
your mesh. These can be created and assigned in your mesh building program.

By default, MOOSE will construct a node set corresponding to each side set within your mesh. This means that if you always prefer
side sets (to node sets) you won't have any issues applying any kind of boundary condition or other "boundary" type object within
MOOSE.

### More detail id=more_detail

As users of MOOSE, you do have the ability to control whether side sets or node sets are automatically constructed. By default
node sets are constructed from side sets but the converse is not true. Both of these parameters can be controlled by the following
parameters (respectively)

```
Mesh/construct_node_list_from_side_list=true
Mesh/construct_side_list_from_node_list=false
```

If you have a mesh that is missing a side set (but it has a node set) you could set
the second parameter (from above) to true.

You could also use the [`ConvertNodeSetSideSetGenerator`](ConvertNodeSetSideSetGenerator.md) class to construct side sets from node
 sets. The `ConvertNodeSetSideSetGenerator` object allows you to manually construct side sets from node sets after mesh generation.

## Minimal interface

When inheriting from MooseMesh, the developer is responsible for creating the mesh and cloning the mesh (used for displaced mesh problems).
The methods that need to be overridden are "buildMesh" and "safeClone".

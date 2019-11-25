# GridPartitioner

!syntax description /Mesh/Partitioner/GridPartitioner

## Description

Partitions the mesh by creating a grid in `x,y,z` and assigning each element of the mesh to a separate "cell" within the grid.  This is useful when you have simple cartesian meshes and you just want to specify partitioning fairly directly.  Sometimes a human is the best partitioner!

This is an example of a `2x2` grid partitioning for use on `4` processors.

!media media/partitioner/grid_partitioner_example.png style=width:40% caption=2x2 `GridPartitioner` Example

!alert warning
The number of cells (`nx*ny*nz`) MUST be equal to the number of MPI processes you're attempting to use!

## How it Works

The GridPartitioner works by creating a [/GeneratedMesh.md] for the "grid".  That's the reason why this object takes similar input file parameters to a [/GeneratedMesh.md].  The GeneratedMesh created by GridPartitioner is guaranteed to contain the original domain within it.

To assign the processor IDs the centroid of each element of the mesh to be partitioned is searched for in the GeneratedMesh.  The ID of the element of the GeneratedMesh that it lies within is then assigned as the `processor_id`.

!syntax parameters /Mesh/Partitioner/GridPartitioner

!syntax inputs /Mesh/Partitioner/GridPartitioner

!syntax children /Mesh/Partitioner/GridPartitioner

!bibtex bibliography

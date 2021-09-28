# HierarchicalGridPartitioner

!syntax description /Mesh/Partitioner/HierarchicalGridPartitioner

## Description

The HierarchicalGridPartitioner is a two-level partitioner similar to [GridPartitioner.md].  The idea is to use a coarse grid with the number of computational nodes (nodes in your cluster you are going to use) to first partition the domain.  Then use a finer-grained grid within each of those partitions to partition for each processor within the computational node.

This type of scheme minimizes off-node communication, minimizing network communication during large simulations.

## Example

An example is the best way to explain what's going on.  The mesh in [hier_mesh] is the mesh we want to partition.  It has 128x128 elements in it (16,384 total).  We're going to be running on a cluster where we're going to use 4 computational nodes---each of which has 16 processors (64 processors total).

As shown in [hier_grid] utilizing [GridPartitioner.md] we can get a decent partitioning of this mesh by specifying the partitioning grid to be 8x8.

Now, the "problem" with this partitionining is that it will do quite a bit of off-processor communication.  Consider what's on the second node (the third and fourth rows from the bottom).  In total there will be 2x128=256 element faces with off-processor neighbors (everything above and below those two rows).  In this case that isn't even that bad.  If we were using even more processors (say 128) then each node would have a long "strip" of partitions on it where each partition would communicate both above and below it.

To fix this we can use the HierarchicalGridPartitioner using syntax shown in [hier_syntax].  By telling it that we are going to have a 2x2 arrangement of nodes and then a 4x4 arrangement of processors on each node we get [hier_pid] showing the processor assignment.  Now each node only has 128 element faces with off-processor neighbors.  Even better, there are a number of partitions that will not do any off-processor communication at all (the four interior ones in the middle of each node).  Even in this small example we've cut the off-processor communication in half by using better partitioning!  In a much larger run with larger numbers of processors per computational node (say 36, 40, 64 or even 128 which we'll see soon) this can make an even bigger difference.

!listing hierarchical_grid_partitioner.i start=Mesh end=Variables id=hier_syntax

!media media/hier_mesh.png style=width:30%;float:left id=hier_mesh caption=Original 128x128 Mesh

!media media/hier_grid.png style=width:30%;float:left id=hier_grid caption=Partitioned using [GridPartitioner](/GridPartitioner.md)

!media media/hier_pid.png style=width:30%;float:left id=hier_pid caption=Partitioned using HierarchicalGridPartitioner


!syntax parameters /Mesh/Partitioner/HierarchicalGridPartitioner

!syntax inputs /Mesh/Partitioner/HierarchicalGridPartitioner

!syntax children /Mesh/Partitioner/HierarchicalGridPartitioner

!bibtex bibliography

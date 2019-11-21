# RandomPartitioner

!syntax description /Mesh/Partitioner/RandomPartitioner

## Description

Partitions the mesh randomly.

This is useful when you want to show the importance of a good partitioner by showing the results with a *very* bad partitioner!

## How it Works

For each element, the RandomPartitioner scales a random number on `[0, 1]` to the number of processors in the partition and assigns the processor ID of the element to this value.

!syntax parameters /Mesh/Partitioner/RandomPartitioner

!syntax inputs /Mesh/Partitioner/RandomPartitioner

!syntax children /Mesh/Partitioner/RandomPartitioner

!bibtex bibliography

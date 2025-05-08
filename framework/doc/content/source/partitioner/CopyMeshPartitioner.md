# CopyMeshPartitioner

This partitioner is used to match the partitioning between two meshes.

Generally, the source mesh should be partitioned with the same amount or less partitions than the target mesh is
intended to be partitioned into. If the source mesh is partitioned with less partitions than the target mesh, some processes will not be
assigned any element in the target mesh, leading to lesser efficiency. If the source mesh is partitioned with more partitions than the target mesh, the partitioning may only succeed if no more than the number of target partitions are used when matching the two meshes.

!syntax parameters /Mesh/Partitioner/CopyMeshPartitioner

!syntax inputs /Mesh/Partitioner/CopyMeshPartitioner

!syntax children /Mesh/Partitioner/CopyMeshPartitioner

!bibtex bibliography

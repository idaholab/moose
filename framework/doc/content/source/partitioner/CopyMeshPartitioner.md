# CopyMeshPartitioner

This partitioner is used to match the partitioning between two meshes.

Generally, the source mesh should be partitioned with the same amount or fewer partitions than the target mesh is
intended to be partitioned into. If the source mesh is partitioned with fewer partitions than the target mesh, some processes will not be
assigned any element in the target mesh, leading to lesser efficiency.

!alert note
If the source mesh is partitioned with more partitions than the target mesh, the partitioner will currently error.

!syntax parameters /Mesh/Partitioner/CopyMeshPartitioner

!syntax inputs /Mesh/Partitioner/CopyMeshPartitioner

!syntax children /Mesh/Partitioner/CopyMeshPartitioner

!bibtex bibliography

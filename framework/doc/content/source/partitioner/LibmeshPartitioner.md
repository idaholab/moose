# LibmeshPartitioner

!syntax description /Mesh/Partitioner/LibmeshPartitioner

The libmesh partitioners available are:

- `METIS`, which uses the [METIS library](http://glaros.dtc.umn.edu/gkhome/metis/metis/overview) for graph partitioning

- `ParMETIS`, which uses the [parallel METIS library](http://glaros.dtc.umn.edu/gkhome/metis/parmetis/overview) for graph partitioning

- `linear` partitioner, which partitions elements based solely on their ids.

- `centroid` partitioner, which partitions based on the element centroids.
  An ordering relation must be created to order the element centroids

- `hilbert_sfc` partitioner which uses Hilbert's space filling curve algorithm

- `morton_sfc` partitioner which uses Morton's space filling curve algorithm

- `subdomain partitioner`, which partitions using the element subdomains


!alert note
The `LibmeshPartitioner` partitions the mesh, not the numerical system. If parts of the mesh
have more variables/DoFs than others, this may cause imbalance.

## Example input syntax

In this example, a `LibmeshPartitioner` is used to perform linear partitioning of the mesh.

!listing test/tests/mesh/custom_partitioner/custom_linear_partitioner_test.i block=Mesh

!syntax parameters /Mesh/Partitioner/LibmeshPartitioner

!syntax inputs /Mesh/Partitioner/LibmeshPartitioner

!syntax children /Mesh/Partitioner/LibmeshPartitioner

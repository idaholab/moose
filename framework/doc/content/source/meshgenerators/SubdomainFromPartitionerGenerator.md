# SubdomainFromPartitionerGenerator

!syntax description /Mesh/SubdomainFromPartitionerGenerator

Note that most partitioners do not guarantee a contiguous partition!
See the [Partitioner syntax page](syntax/Mesh/Partitioner/index.md) for a list of partitioners available in MOOSE.

!alert note
Most partitioners will error if the number of parallel processes does not match
the number of partitions requested. This mesh generator may be used in parallel,
preceded by a [FileMeshGenerator.md] if needed to load a starting mesh previously generated in serial,
then its output may be loaded by other mesh generators, again using a [FileMeshGenerator.md]
if these following mesh generators prefer working on a mesh in serial.

!syntax parameters /Mesh/SubdomainFromPartitionerGenerator

!syntax inputs /Mesh/SubdomainFromPartitionerGenerator

!syntax children /Mesh/SubdomainFromPartitionerGenerator

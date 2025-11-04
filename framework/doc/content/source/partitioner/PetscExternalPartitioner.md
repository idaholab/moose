# PetscExternalPartitioner

Allow users to use several external partitioning packages (parmetis, chaco, ptscotch and party) via PETSc.
Note that partitioning, just as meshing, requires a level of user insight to assure the domain is appropriately distributed. For example, edge cases where one seeks to have very few elements per process
may misbehave with certain partitioners. To avert such situations, we switch ParMETIS to PTScotch in cases with less than 28 elements per process, and notify the user.

## [ParMETIS](http://glaros.dtc.umn.edu/gkhome/metis/parmetis/overview)

ParMETIS is an MPI-based parallel graph partitioner implementing mainly a multilevel K-way algorithm.  The basic idea
of the multilevel K-way algorithm is to coarsen the graph firstly, partition the coarsened graph and then refine the partition.
It is solving a multi-constraints optimization problem.

## [PTScotch](https://www.labri.fr/perso/pelegrin/scotch/)

PTScotch is a software package  which compute parallel static mappings and parallel sparse matrix block orderings of graphs. It implements graph bipartitioning methods including band, diffusion and multilevel methods.

## [Chaco](https://www3.cs.stonybrook.edu/~algorith/implement/chaco/implement.shtml)

Chaco contains a wide variety of algorithms and options. Some of the algorithms exploit the geometry of the mesh, others its local connectivity or its global structure as captured by eigenvectors of a related matrix.

## [Party](https://cran.r-project.org/web/packages/party/vignettes/party.pdf)

The party package aims at providing a recursive partitioning laboratory assembling various high- and low-level tools for building tree-based regression and classification models.

## Use

These packages can be accessed via an unified interface in MOOSE, `PetscExternalPartitioner`. The use of the packages is accomplished by adding a subblock in `Mesh` block of input file.  For example

!listing test/tests/partitioners/petsc_partitioner/petsc_partitioner.i block=Mesh

## Partitioning Examples

!alert note
For less than 28 elements per process we switch partitioner from ParMETIS to PTScotch. For these two partitioners we illustrate the refined grid case, where the switch does not occur.

### 4 subdomains

!row!
!col! small=12 medium=6 large=3
10x10 grid

!media party_4parts.png caption=`Party`
!col-end!

!col! small=12 medium=6 large=3
10x10 grid

!media chaco_4parts.png caption=`chaco`
!col-end!

!col! small=12 medium=6 large=3
20x20 grid

!media parmetis20_4P.png caption=`parmetis`
!col-end!

!col! small=12 medium=6 large=3
20x20 grid

!media ptscotch20_4P.png caption=`ptscotch`
!col-end!
!row-end!


### 8 subdomains

!row!
!col! small=12 medium=6 large=3
!media party_8parts.png caption=`Party`
!col-end!

!col! small=12 medium=6 large=3
!media chaco_8parts.png caption=`chaco`
!col-end!

!col! small=12 medium=6 large=3
!media parmetis20_8P.png caption=`parmetis`
!col-end!

!col! small=12 medium=6 large=3
!media ptscotch20_8P.png caption=`ptscotch`
!col-end!
!row-end!


!alert note
By default, all element and face weights are uniform. This can be modified by implementing `computeElementWeight`
and `computeSideWeight` in a derived class of `PetscExternalPartitioner`. For example, the [BlockWeightedPartitioner.md]
returns different weights for all elements in a block.

!syntax parameters /Mesh/Partitioner/PetscExternalPartitioner

!syntax inputs /Mesh/Partitioner/PetscExternalPartitioner

!syntax children /Mesh/Partitioner/PetscExternalPartitioner

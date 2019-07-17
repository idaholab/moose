# BlockWeightedPartitioner

!syntax description /Mesh/Partitioner/BlockWeightedPartitioner

## Overview

Under multiphysics environment, some mesh blocks have more variables and more work load than others .  In parallel, the work
is spread out based on partitioners that assign equal number of elements to each processor. This will causes imbalanced simulations.
BlockWeightedPartitioner allow users to specify different weights for different blocks, e.g., low weights for light blocks
and high weights for heavy blocks. Weights should be proportional to the relative numbers of DOFs per element in each mesh block.
The weights do not need to be pre-normalized to any particular number.  Small numbers are better large numbers, e.g., 1 vs 2
is better than 1000 vs 2000.   Usage:

!listing block_weighted_partitioner.i block=Mesh


An example:

!row!
!col! small=12 medium=6 large=3
!media weightedpartitioner4.png caption=`Weighted partition into 4 subdomains`
!col-end!

!col! small=12 medium=6 large=3
!media noweightedpartitioner4.png caption=`Regular partition into 4 subdomains`
!col-end!
!row-end!


!syntax parameters /Mesh/Partitioner/BlockWeightedPartitioner

!syntax inputs /Mesh/Partitioner/BlockWeightedPartitioner

!syntax children /Mesh/Partitioner/BlockWeightedPartitioner

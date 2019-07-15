
!template load file=stubs/moose_object.md.template name=BlockWeightedPartitioner syntax=/Mesh/Partitioner/BlockWeightedPartitioner

## Overview

Under multiphysics environment, some mesh blocks have more variables and more work load than others .  In parallel, the work
is spread out based on partitioners that assign equal number of elements to each processor. This will causes imbalanced simulations.
BlockWeightedPartitioner allow users to specify different weights for different blocks, e.g., low weights for light blocks
and high weights for heavy blocks.  Usage:

```
[Mesh]
  type = FileMesh
  file = block_weighted_partitioner.e

  [Partitioner]
    type = BlockWeightedPartitioner
    block = '1 2 3'
    weight = '3 1 10'
  []
[]
```

An example:

!row!
!col! small=12 medium=6 large=3
!media weightedpartitioner4.png caption=`Weighted partition into 4 subdomains`
!col-end!

!col! small=12 medium=6 large=3
!media noweightedpartitioner4.png caption=`Regular partition into 4 subdomains`
!col-end!
!row-end!

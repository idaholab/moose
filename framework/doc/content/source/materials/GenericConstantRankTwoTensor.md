# GenericConstantRankTwoTensor

!syntax description /Materials/GenericConstantRankTwoTensor

## Overview

`GenericConstantRankTwoTensor` creates a `RankTwoTensor` material property that use
constant values to fill the tensor. The input of the constants should be column major-ordered.

This can be used to quickly create simple constant tensor material properties, for testing,
for initial survey of a problem or simply because the material properties do not vary much over the
domain explored by the simulation.

## Example Input File Syntax

!listing test/tests/materials/generic_materials/generic_constant_rank_two_tensor.i block=Materials/tensor

!syntax parameters /Materials/GenericConstantRankTwoTensor

!syntax inputs /Materials/GenericConstantRankTwoTensor

!syntax children /Materials/GenericConstantRankTwoTensor

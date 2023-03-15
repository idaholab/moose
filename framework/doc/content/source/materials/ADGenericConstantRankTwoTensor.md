# ADGenericConstantRankTwoTensor

!syntax description /Materials/GenericConstantRankTwoTensor

## Overview

`ADGenericConstantRankTwoTensor` creates a `RankTwoTensor` material property that use
constant values to fill the tensor. The input of the constants should be column major-ordered.

## Example Input File Syntax

!listing test/tests/materials/generic_materials/ad_generic_constant_rank_two_tensor.i block=Materials/tensor

!syntax parameters /Materials/ADGenericConstantRankTwoTensor

!syntax inputs /Materials/ADGenericConstantRankTwoTensor

!syntax children /Materials/ADGenericConstantRankTwoTensor

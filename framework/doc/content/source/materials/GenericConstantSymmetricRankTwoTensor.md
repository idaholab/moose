# GenericConstantSymmetricRankTwoTensor

!syntax description /Materials/GenericConstantSymmetricRankTwoTensor

## Overview

`GenericConstantSymmetricRankTwoTensor` creates a `SymmetricRankTwoTensor` material property that use
constant values to fill the tensor. The input of the constants follows the way how a `SymmetricRankTwoTensor` is filled.

This object functions similarly to the [GenericConstantRankTwoTensor](GenericConstantRankTwoTensor.md), but it leverages the symmetry property of the tensor for more efficient computation.

## Example Input File Syntax

!listing test/tests/materials/generic_materials/generic_constant_rank_two_tensor.i block=Materials/tensor

!syntax parameters /Materials/GenericConstantSymmetricRankTwoTensor

!syntax inputs /Materials/GenericConstantSymmetricRankTwoTensor

!syntax children /Materials/GenericConstantSymmetricRankTwoTensor

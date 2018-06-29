# Line Material Rank Two Sampler

!syntax description /VectorPostprocessors/LineMaterialRankTwoSampler

## Description

The postprocessor `LineMaterialRankTwoSampler` is used to output specific components of Rank-2 tensors along a user-defined line in the mesh.
The postprocessor uses indices, similar to the AuxKernel [RankTwoAux](/RankTwoAux.md), to determine the component of the Rank-2 tensor to output along a line.

The user must supply the start and end points of the line along which the Rank-2 tensor component should be tracked.
Often this class is used to track stress or strain along an exterior or interior edge of the mesh.

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/line_material_rank_two_sampler/rank_two_sampler.i block=VectorPostprocessors/stress_xx

!syntax parameters /VectorPostprocessors/LineMaterialRankTwoSampler

!syntax inputs /VectorPostprocessors/LineMaterialRankTwoSampler

!syntax children /VectorPostprocessors/LineMaterialRankTwoSampler

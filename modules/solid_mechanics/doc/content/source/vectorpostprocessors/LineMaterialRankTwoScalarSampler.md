# Line Material Rank Two Scalar Sampler

!syntax description /VectorPostprocessors/LineMaterialRankTwoScalarSampler

## Description

The postprocessor `LineMaterialRankTwoScalarSampler` is used to output common scalar quantities computed from Rank-2 tensors along a user-defined line in the mesh.
The postprocessor computes the same set of scalar quantities as the AuxKernel [RankTwoScalarAux](/RankTwoScalarAux.md); for a full list of the available scalar quantities refer to the [RankTwoScalarAux](/RankTwoScalarAux.md) page.

The user must supply the start and end points of the line along which the Rank-2 tensor scalar quantity should be tracked.
Often this class is used to track stress or strain along an exterior or interior edge of the mesh.

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/line_material_rank_two_sampler/rank_two_scalar_sampler.i block=VectorPostprocessors/vonmises

!syntax parameters /VectorPostprocessors/LineMaterialRankTwoScalarSampler

!syntax inputs /VectorPostprocessors/LineMaterialRankTwoScalarSampler

!syntax children /VectorPostprocessors/LineMaterialRankTwoScalarSampler

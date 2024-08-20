# AdjointStrainStressGradInnerProduct

This component is designed to compute the gradient of the objective function concerning specific properties. It achieves this by computing the inner product of the property derivative (`SymmetricRankTwoTensor`) and the strain (`RankTwoTensor`) resulting from the forward simulation.

## Example Input Syntax

!syntax parameters /VectorPostprocessors/AdjointStrainStressGradInnerProduct

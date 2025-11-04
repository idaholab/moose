# AdjointStrainSymmetricStressGradInnerProduct

This object is designed to compute the gradient of the objective function concerning specific properties. It achieves this by computing the inner product of the property derivative (`SymmetricRankTwoTensor`) and the strain (`RankTwoTensor`) resulting from the forward simulation.

## Example Input Syntax

!listing test/tests/optimization/invOpt_elasticity_modular/grad.i block=VectorPostprocessors/grad_youngs_modulus

!syntax parameters /VectorPostprocessors/AdjointStrainSymmetricStressGradInnerProduct

!syntax inputs /VectorPostprocessors/AdjointStrainSymmetricStressGradInnerProduct

!syntax children /VectorPostprocessors/AdjointStrainSymmetricStressGradInnerProduct

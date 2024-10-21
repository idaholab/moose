# AdjointStrainSymmetricStressGradInnerProduct

This object is designed to compute the gradient of the objective function concerning specific properties. It achieves this by computing the inner product of the property derivative (`SymmetricRankTwoTensor`) and the strain (`RankTwoTensor`) resulting from the forward simulation.

## Example Input Syntax

!listing test/tests/vectorpostprocessors/adjoint_strain_batch_stress_grad_inner_product/strain_stress_grad_inner_product_material.i block=VectorPostprocessors/inner_product

!syntax parameters /VectorPostprocessors/AdjointStrainSymmetricStressGradInnerProduct

!syntax inputs /VectorPostprocessors/AdjointStrainSymmetricStressGradInnerProduct

!syntax children /VectorPostprocessors/AdjointStrainSymmetricStressGradInnerProduct

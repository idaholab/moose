# MaterialDerivativeRankTwoTestKernel

!syntax description /Kernels/MaterialDerivativeRankTwoTestKernel

This kernel puts a seleted tensorial (`RankTwoTensor` type) material property (`material_property`) in the residual vector
and assembles the Jacobian using the derivatives of the material property as provided by the
[DerivativeMaterialInterface](/DerivativeMaterialInterface.md).

This kernel is best set up using the [MaterialDerivativeTest](/MaterialDerivativeTestAction.md) action.

!syntax parameters /Kernels/MaterialDerivativeRankTwoTestKernel

!syntax inputs /Kernels/MaterialDerivativeRankTwoTestKernel

!syntax children /Kernels/MaterialDerivativeRankTwoTestKernel

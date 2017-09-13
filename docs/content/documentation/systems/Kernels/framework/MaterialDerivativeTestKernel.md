# MaterialDerivativeTestKernel
!syntax description /Kernels/MaterialDerivativeTestKernel

This kernel puts a seleted scalar (`Real` type) material property (`material_property`) in the residual vector
and assembles the Jacobian using the derivatives of the material property as provided by the
[DerivativeMaterialInterface](/DerivativeMaterialInterface.md).

!syntax parameters /Kernels/MaterialDerivativeTestKernel

!syntax inputs /Kernels/MaterialDerivativeTestKernel

!syntax children /Kernels/MaterialDerivativeTestKernel

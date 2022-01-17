# MaterialDerivativeStdVectorRealTestKernel

This kernel is used to obtain derivatives of quantities that depend on `aXrhoA_vapor` variable.
These derivatives have `std::vector<Real>` type so the standard `MaterialDerivativeTestKernel`
cannot be used.

However, this kernel should be used *only* with conjunction of `MaterialDerivativeTestKernel`, because
that kernel provides the correct residual and this kernel only adds the derivatives.

!syntax description /Kernels/MaterialDerivativeStdVectorRealTestKernel

!syntax parameters /Kernels/MaterialDerivativeStdVectorRealTestKernel

!syntax inputs /Kernels/MaterialDerivativeStdVectorRealTestKernel

!syntax children /Kernels/MaterialDerivativeStdVectorRealTestKernel

!bibtex bibliography

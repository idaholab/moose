# Hybridized Boundary Conditions

Hybridized boundary conditions implement boundary conditions for physics
implemented in [HybridizedKernels/index.md]. All hybridized boundary conditions
should inherit from `HybridizedIntegratedBC` as at the present time all
hybridized boundary conditions are imposed weakly. Classes derived from
`HybridizedIntegratedBC` must implement the `onBoundary` virtual method. Like
their `HybridizedKernel` counterparts, hybridized boundary conditions should
populate the data members `_PrimalMat`, `_LMMat`, `_PrimalLM`, `_LMPrimal`,
`_PrimalVec`, and `_LMVec`.

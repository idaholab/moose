# HDG Boundary Conditions

HDG boundary conditions implement boundary conditions for physics
implemented in [HDGKernels/index.md]. All hybridized boundary conditions
should inherit from `HDGIntegratedBC` as at the present time all
hybridized boundary conditions are imposed weakly. Classes derived from
`HDGIntegratedBC` must implement the `onBoundary` virtual method. Like
their `HDGKernel` counterparts, hybridized boundary conditions should
populate the data members `_PrimalMat`, `_LMMat`, `_PrimalLM`, `_LMPrimal`,
`_PrimalVec`, and `_LMVec`.

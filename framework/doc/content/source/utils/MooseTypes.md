# MooseTypes

The `MooseTypes` header contains several names central to automatic
differentiation in MOOSE. It defines the following macros:

- `ADReal`
- `ADRealVectorValue`
- `ADPoint`
- `ADRealTensorValue`
- `ADRankTwoTensor`
- `ADRankFourTensor`

which resolve to either `Real` or [`DualReal`](/DualReal.md) based objects
depending on whether `compute_stage` is equivalent to `RESIDUAL` or
`JACOBIAN`. `compute_stage` is a template argument to AD consuming and producing
objects like `ADKernel` and `ADMaterial` respectively.

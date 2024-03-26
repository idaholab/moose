# AdjointStrainBatchStressGradInnerProduct

This component is designed to compute the gradient of the objective function concerning specific properties. It achieves this by computing the inner product of the property derivative and the strain resulting from the forward simulation. The property derivative is extracted from a batch material of the [BatchPropertyDerivative](BatchPropertyDerivative.md) type.

## Example Input Syntax

!syntax parameters /VectorPostprocessors/AdjointStrainBatchStressGradInnerProduct

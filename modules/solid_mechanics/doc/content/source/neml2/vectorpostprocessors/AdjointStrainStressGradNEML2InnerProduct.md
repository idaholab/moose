# AdjointStrainStressGradNEML2InnerProduct

!syntax description /VectorPostprocessors/AdjointStrainStressGradNEML2InnerProduct

## Description

This component is designed to compute the gradient of the objective function concerning specific properties. It achieves this by computing the inner product of the property derivative obtained from a NEML2 material model and the strain resulting from the forward simulation. The property derivative is extracted from a batch material of the [BatchPropertyDerivative](BatchPropertyDerivative.md) type.

## Example Input Syntax

!syntax parameters /VectorPostprocessors/AdjointStrainStressGradNEML2InnerProduct

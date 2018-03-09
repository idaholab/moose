# Function Materials

Function materials describe a family of [Moose material classes](/Materials/index.md) that are
derived from the `FunctionMaterialBase` base class. A function material provides a `Real` material
property that hold the value of an arbitrary function expression. A derivative function material in
addition provides further material properties that hold the value of the derivatives of the function
expression with respect to each coupled variable up to a defined `derivative_order`.

Available function materials are

| Material | Description |
| - | - |
| [`ParsedMaterial`](/ParsedMaterial.md) | Sets up a single material property that is computed using a parsed function expression |
| [`DerivativeParsedMaterial`](/DerivativeParsedMaterial.md) | Extends `ParsedMaterial` and provides the automatically generated derivatives of the function expression w.r.t. all coupled variables. |
| [`DerivativeSumMaterial`](/DerivativeSumMaterial.md) | Create new material properties that contain the sum of multiple function material properties (including the sums of the derivatives) |
| [`ElasticEnergyMaterial`](/ElasticEnergyMaterial.md) | Generates an elastic free energy function material from the current stress/strain state including necessary derivatives |
| [`SwitchingFunctionMaterial`](/SwitchingFunctionMaterial.md) | A convenience material to generate a soft switching function used in multiphase modeling |
| [`BarrierFunctionMaterial`](/BarrierFunctionMaterial.md) | A convenience material to generate a barrier function used in multiphase modeling |
| [`MathFreeEnergy`](/MathFreeEnergy.md) | A simple double-well free energy $F = \frac14(1 + c)^2(1 - c)^2$ (and its concentration derivatives) |


## See also

- [Automatic Differentiation](FunctionMaterials/AutomaticDifferentiation.md) for MOOSE application developers

# Function Material Kernels

Kernels [`CahnHilliard`](/Kernels/CahnHilliard.md), [`CahnHilliardAniso`](/Kernels/CahnHilliardAniso.md),
[`SplitCHParsed`](/Kernels/SplitCHParsed.md), and [`AllenCahn`](/Kernels/AllenCahn.md) solve
the Cahn-Hilliard and Allen-Cahn equations using  free energies provided by
[Function Material](../FunctionMaterials) objects. These include parsed function
materials with free energy expressions supplied in the configuration files and all
necessary derivatives to build the _Residuals_ and _Jacobian_ elements computed
automatically using automatic differentiation
([`DerivativeParsedMaterial`](/Materials/DerivativeParsedMaterial.md)).

This enables rapid implementation of new Phase Field models without writing custom kernels and recompiling the code.

## See also

* [Automatic Differentiation](AutomaticDifferentiation) for MOOSE application developers
* [ExpressionBuilder](ExpressionBuilder) - building FParser expressions at compile time using operator overloading

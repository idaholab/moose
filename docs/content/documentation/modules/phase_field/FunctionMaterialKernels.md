# Function Material Kernels

Kernels [`CahnHilliard`](/CahnHilliard.md), [`CahnHilliardAniso`](/CahnHilliardAniso.md),
[`SplitCHParsed`](/SplitCHParsed.md), and [`AllenCahn`](/AllenCahn.md) solve
the Cahn-Hilliard and Allen-Cahn equations using  free energies provided by
[Function Material](../FunctionMaterials) objects. These include parsed function
materials with free energy expressions supplied in the configuration files and all
necessary derivatives to build the _Residuals_ and _Jacobian_ elements computed
automatically using automatic differentiation
([`DerivativeParsedMaterial`](/DerivativeParsedMaterial.md)).

This enables rapid implementation of new Phase Field models without writing custom kernels and recompiling the code.

## See also

* [Automatic Differentiation](AutomaticDifferentiation) for MOOSE application developers
* [ExpressionBuilder](ExpressionBuilder) - building FParser expressions at compile time using operator overloading

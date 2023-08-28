# ParsedFunctorMaterial

This [functor material](/FunctorMaterials/index.md) computes a functor material
property using a mathematical expression provided by a string. The expression
may operate on any of the following values:

- the spatial point, provided by `x`, `y`, and `z`,
- the time, provided by `t`, and
- any [functor](/Functors/index.md).

## Usage

This functor material creates a functor material property with the name given
by [!param](/FunctorMaterials/ParsedFunctorMaterial/property_name).

The mathematical expression is provided via [!param](/FunctorMaterials/ParsedFunctorMaterial/expression).

Functors used in this expression are provided via [!param](/FunctorMaterials/ParsedFunctorMaterial/functor_names).
These functors can optionally be assigned alternate (usually simpler) names
for use in the expression by providing [!param](/FunctorMaterials/ParsedFunctorMaterial/functor_symbols).
Otherwise their names are used directly in the expression.

For more information on the mathematical operators that can be used in the
expression, see the [function parser site](http://warp.povusers.org/FunctionParser/).

!syntax parameters /FunctorMaterials/ParsedFunctorMaterial

!syntax inputs /FunctorMaterials/ParsedFunctorMaterial

!syntax children /FunctorMaterials/ParsedFunctorMaterial

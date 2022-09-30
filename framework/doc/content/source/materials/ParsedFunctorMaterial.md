# ParsedFunctorMaterial

!syntax description /Materials/ParsedFunctorMaterial

Sets up a single functor material property that is computed using a parsed function expression.
This is the equivalent of the [ParsedMaterial.md] but as a functor material property instead of a
regular material property.

A `ParsedFunctorMaterial` object takes the function expression as an input parameter in
the form of a Function Parser expression. Parsed materials (unlike
`ParsedFunctions`) can couple to nonlinear variables.
In its configuration block all nonlinear variables the function depends on
([!param](/Materials/ParsedFunctorMaterial/args)), as well as constants
([!param](/Materials/ParsedFunctorMaterial/constant_names) and
[!param](/Materials/ParsedFunctorMaterial/constant_expressions)), other functors
([!param](/Materials/ParsedFunctorMaterial/functor_names)), and
postprocessors ([!param](/Materials/ParsedFunctorMaterial/postprocessor_names)) are
declared. Constants can be declared as parsed expressions (which can depend on
previously defined constants).

## Example

The following material object creates a single property that depends on a function,
a postprocessor, an auxiliary variable, another functor material property and two nonlinear
variables. While all variables are functors, one of the nonlinear variables is passed
as a variable to take advantage of some parsed expression additional parameters.

!listing test/tests/materials/parsed_functor_material/test.i block=Materials

!syntax parameters /Materials/ParsedFunctorMaterial

!syntax inputs /Materials/ParsedFunctorMaterial

!syntax children /Materials/ParsedFunctorMaterial

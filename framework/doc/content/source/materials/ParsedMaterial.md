# ParsedMaterial

!syntax description /Materials/ParsedMaterial

Sets up a single material property that is computed using a parsed function expression.

A `ParsedMaterial` object takes the function expression as an input parameter in
the form of a Function Parser expression. Parsed materials (unlike
`ParsedFunctions`) can couple to nonlinear variables and material properties.
In its configuration block all nonlinear variables the function depends on
([!param](/Materials/ParsedMaterial/coupled_variables)), as well as constants
([!param](/Materials/ParsedMaterial/constant_names) and
[!param](/Materials/ParsedMaterial/constant_expressions)), other material
properties ([!param](/Materials/ParsedMaterial/material_property_names)), and
postprocessors ([!param](/Materials/ParsedMaterial/postprocessor_names)) are
declared. Constants can be declared as parsed expressions (which can depend on
previously defined constants). One application would be the definition of a
temperature $T$, the Boltzmann constant $k_B$, a defect formation energy $E_F$,
and then an equilibrium defect concentration defined using a Boltzmann factor
$\exp(-\frac{E_d}{k_BT})$.

## Example

The following material object creates a single property for visualization purposes.
It will be 0 for phase 1, -1 for phase 2, and 1 for phase 3

!listing modules/combined/examples/phase_field-mechanics/Pattern1.i start=phasemap end=matrix

!syntax parameters /Materials/ParsedMaterial

!syntax inputs /Materials/ParsedMaterial

!syntax children /Materials/ParsedMaterial

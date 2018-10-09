# ElementalVariableValue

!syntax description /Postprocessors/ElementalVariableValue

## Description

In some cases it may be of interest to output an elemental variable value (e.g., stress)
at a particular location in the model.  This is accomplished by using the
`ElementalVariableValue` postprocessor. This postprocessor takes a specific element ID
to sample (only a single ID). The value of the specified variable is integrated over the
element and then returned.

## Example Input Syntax

!listing test/tests/outputs/postprocessor/output_pps_hidden_shown_check.i block=Postprocessors

!syntax parameters /Postprocessors/ElementalVariableValue

!syntax inputs /Postprocessors/ElementalVariableValue

!syntax children /Postprocessors/ElementalVariableValue

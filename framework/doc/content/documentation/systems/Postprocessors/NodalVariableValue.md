# NodalVariableValue

!syntax description /Postprocessors/NodalVariableValue

## Description

In order to obtain the value of a nodal variable at a particular location (i.e.,
temperature and displacement) a `NodalVariableValue` postprocessor is used. For example,
this postprocessor is useful for obtaining the centerline temperature at the location of a
thermocouple to compare against experimental data.

## Example Input Syntax

!listing test/tests/misc/check_error/nodal_value_off_block.i block=Postprocessors

!syntax parameters /Postprocessors/NodalVariableValue

!syntax inputs /Postprocessors/NodalVariableValue

!syntax children /Postprocessors/NodalVariableValue

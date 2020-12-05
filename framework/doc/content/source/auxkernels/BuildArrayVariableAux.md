# BuildArrayVariableAux

## Description

`BuildArrayVariableAux` will fill the values of an array variable using a
standard variable for each of the array components. Currently this aux kernel
only supports constant monomial variables.

## Example Syntax

!listing /test/tests/auxkernels/build_array_variable_aux/build_array_variable_aux.i block=Variables

!listing /test/tests/auxkernels/build_array_variable_aux/build_array_variable_aux.i block=AuxVariables

!listing /test/tests/auxkernels/build_array_variable_aux/build_array_variable_aux.i block=AuxKernels

!syntax parameters /AuxKernels/BuildArrayVariableAux

!syntax inputs /AuxKernels/BuildArrayVariableAux

!syntax children /AuxKernels/BuildArrayVariableAux

# FunctionArrayAux

## Description

`FunctionArrayAux` is used to evaluate an auxiliary array variable with a set of functions.
The number of functions is equal to the number of components of the array variable.
For a nodal array variable, function values at the node points are assigned to the components of the array variable.
For an elemental array variable, element-wise L2 projection is performed for all the components with the function values evaluated on the element quadrature points.

!syntax parameters /AuxKernels/FunctionArrayAux

!syntax inputs /AuxKernels/FunctionArrayAux

!syntax children /AuxKernels/FunctionArrayAux

# ElementL2ErrorFunctionAux

A class for computing the element-wise $L^2$ error (actually $L^p$ error, if you set the value of p to something other than 2) of the difference between an exact solution (typically represented by a [ParsedFunction](/MooseParsedFunction.md)) and the coupled solution variable.  The base class implements the compute() function.

## Example syntax

!listing test/tests/auxkernels/error_function_aux/error_function_aux.i block=AuxKernels/l2_error_aux

!syntax description /AuxKernels/ElementL2ErrorFunctionAux

!syntax parameters /AuxKernels/ElementL2ErrorFunctionAux

!syntax inputs /AuxKernels/ElementL2ErrorFunctionAux

!syntax children /AuxKernels/ElementL2ErrorFunctionAux

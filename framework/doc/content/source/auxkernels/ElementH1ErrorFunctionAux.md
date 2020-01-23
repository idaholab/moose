# ElementH1ErrorFunctionAux

A class for computing the element-wise $H1$ (Sobolev space) error (actually $W^{1,p}$ error, if you set the value of $p$ to something other than 2.0) of the difference between an exact solution (typically represented by a [ParsedFunction](/MooseParsedFunction.md)) and the specified solution variable.

## Example syntax

!listing test/tests/auxkernels/error_function_aux/error_function_aux.i block=AuxKernels/h1_error_aux

!syntax description /AuxKernels/ElementH1ErrorFunctionAux

!syntax parameters /AuxKernels/ElementH1ErrorFunctionAux

!syntax inputs /AuxKernels/ElementH1ErrorFunctionAux

!syntax children /AuxKernels/ElementH1ErrorFunctionAux

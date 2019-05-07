# VectorFunctionAux

!syntax description /AuxKernels/VectorFunctionAux

## Example

The `VectorFunctionAux` object may be used to populate a vector auxiliary variable for use with
objects designed to be coupled to vector variables or for computing vector valued values such
as velocity for post processing purposes.

The following partial input file shows an example of populating a vector auxiliary variable
using a parsed function.

!listing vector_function_aux.i block=AuxVariables Functions AuxKernels


!syntax parameters /AuxKernels/VectorFunctionAux

!syntax inputs /AuxKernels/VectorFunctionAux

!syntax children /AuxKernels/VectorFunctionAux

!bibtex bibliography

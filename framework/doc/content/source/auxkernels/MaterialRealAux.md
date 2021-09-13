# MaterialRealAux

!syntax description /AuxKernels/MaterialRealAux

## Description

The `MaterialRealAux` AuxKernel is used to output material properties as an element-level,
constant variable. The computed value will be the volume-averaged quantity over the element.

Converting a field from the material system, here a component of a matrix material property,
to a variable may be desirable for several reasons: to match the format expected by certain
kernels (thus lagging the field between time steps) or for output/testing/debugging.

!alert note
The AD system currently does not support auxiliary variables. If you convert material properties, which
do support automatic differentiation, to auxiliary variables, the derivatives will be ignored.

## Example Input Syntax

!listing test/tests/materials/boundary_material/elem_aux_bc_on_bnd.i block=AuxKernels

!syntax parameters /AuxKernels/MaterialRealAux

!syntax inputs /AuxKernels/MaterialRealAux

!syntax children /AuxKernels/MaterialRealAux

# MaterialRealAux
!syntax description /AuxKernels/MaterialRealAux

## Description
The `MaterialRealAux` AuxKernel is used to output material properties as a element-level,
constant variable. The computed value will be the volume-averaged quantity over the element.

## Example Input Syntax
!listing test/tests/materials/boundary_material/elem_aux_bc_on_bnd.i block=AuxKernels

!syntax parameters /AuxKernels/MaterialRealAux

!syntax inputs /AuxKernels/MaterialRealAux

!syntax children /AuxKernels/MaterialRealAux

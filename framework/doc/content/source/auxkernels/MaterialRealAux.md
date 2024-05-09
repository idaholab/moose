# MaterialRealAux

!syntax description /AuxKernels/MaterialRealAux

## Description

The `MaterialRealAux` AuxKernel is used to output material properties as an element-level,
variable. When providing a constant monomial variable the computed value will be the
volume-averaged quantity over the element.

The variant `FunctorMaterialRealAux` can output a functor property to either a nodal or an
elemental variable.

Converting a field from the material system, here a component of a matrix material property,
to a variable may be desirable for several reasons: to match the format expected by certain
kernels (thus lagging the field between time steps) or for output/testing/debugging.

!alert note
The AD system currently does not support auxiliary variables. If you convert material properties
or functors, which do support automatic differentiation, to auxiliary variables, the derivatives
will be ignored.

## Example Input Syntax

### Material property

!listing test/tests/materials/boundary_material/elem_aux_bc_on_bnd.i block=AuxKernels

### Functor

!listing test/tests/auxkernels/functor_material_aux/real.i block=AuxKernels

!syntax parameters /AuxKernels/MaterialRealAux

!syntax inputs /AuxKernels/MaterialRealAux

!syntax children /AuxKernels/MaterialRealAux

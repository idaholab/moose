# NodalNormalAux

## Description
!syntax description /AuxKernels/NodalNormalAux

## Example Input File Syntax

!listing /test/tests/bcs/nodal_normals/square_quads.i id=example caption=Example of nodal normals visualization start=NodalNormals end=Executioner

In this example, the simulation is in two dimensions, so we set the z-coordinate to 0 using
`ConstantAux`.

!syntax parameters /AuxKernels/NodalNormalAux

!syntax children /AuxKernels/NodalNormalAux

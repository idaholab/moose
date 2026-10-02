# ADGlobalDisplacementAux

!syntax description /AuxKernels/ADGlobalDisplacementAux

## Description

The `ADGlobalDisplacementAux` constructs
\begin{equation}
\bold{u} = \epsilon(\bold{X} - \bold{X^o})
\end{equation}
where:

$\bold{u}$ = Displacement from global strain.

$\bold{X}$ = Coordinate vector of a node.

$\bold{X^o}$ = Reference coordinate.

One AuxKernel is needed for every displacement direction within a mesh and every auxillary displacement variable. I.e. `disp_x` corresponds to component 0, `disp_y` to 1, and `disp_z` to 2.

This assignment is done for output and post-processing purposes.

## AD Global Strain Objects

Used with the automatic differentiation version of Global Strain.

[ADGlobalStrain](/ADGlobalStrain.md)

[ADComputeGlobalStrain](/ADComputeGlobalStrain.md)

[GlobalStrainPeriodicDirUserObject](/GlobalStrainPeriodicDirUserObject.md)

!syntax parameters /AuxKernels/ADGlobalDisplacementAux

!syntax inputs /AuxKernels/ADGlobalDisplacementAux

!syntax children /AuxKernels/ADGlobalDisplacementAux

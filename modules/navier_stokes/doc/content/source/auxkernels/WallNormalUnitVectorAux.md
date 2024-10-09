# WallNormalUnitVectorAux

Computes a unit normal vector for each cell that points to the nearest wall.
The list of walls to consider when calculating the distance is specified in the [!param](/AuxKernels/WallNormalUnitVectorAux/walls) parameter.

!alert note
The [!param](/AuxKernels/WallNormalUnitVectorAux/walls) parameter cannot be empty.
Otherwise the output of this computation makes no physical sense.

!syntax parameters /AuxKernels/WallNormalUnitVectorAux

!syntax inputs /AuxKernels/WallNormalUnitVectorAux

!syntax children /AuxKernels/WallNormalUnitVectorAux

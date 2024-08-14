# WallFunctionWallShearStressAux

This function computes the wall shear stress obtained using the standard
wall function in [INSFVWallFunctionBC](source/fvbcs/INSFVWallFunctionBC.md). This wall function can be modified with roughness or curvature corrections if needed.
It assumes that the first cell centroid is located in the log layer.

!syntax parameters /AuxKernels/WallFunctionWallShearStressAux

!syntax inputs /AuxKernels/WallFunctionWallShearStressAux

!syntax children /AuxKernels/WallFunctionWallShearStressAux

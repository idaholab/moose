# TotalDisplacementAux

!syntax description /AuxKernels/TotalDisplacementAux

## Description

The AuxKernel `TotalDisplacementAux` is used to calculate the total displacement components combining the values obtained from the global strain tensor and stress divergence kernel. It saves individual displacement components into an AuxVariable for visualization and/or post-processing purposes. For further details about visualizing the deformation with respect to total deformation please refer to [GlobalDisplacementAux](/AuxKernels/GlobalDisplacementAux.md).


## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/global_strain/global_strain.i block=AuxKernels/disp_x

An AuxVariable is required to store the displacements calculated by the AuxKernel.
The name of the AuxVariable is used as the argument for the `variable` input parameter.

!listing modules/tensor_mechanics/test/tests/global_strain/global_strain.i block=AuxVariables/disp_x


!syntax parameters /AuxKernels/TotalDisplacementAux

!syntax inputs /AuxKernels/TotalDisplacementAux

!syntax children /AuxKernels/TotalDisplacementAux

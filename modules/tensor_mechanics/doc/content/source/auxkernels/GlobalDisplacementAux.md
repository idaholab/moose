# GlobalDisplacementAux

!syntax description /AuxKernels/GlobalDisplacementAux

## Description

The AuxKernel `GlobalDisplacementAux` calculates the displacement components from the [global strain tensor](/ComputeGlobalStrain.md). Optionally, it can combine the global displacement with the displacements calculated from the stress divergence kernel to provide the total displacement. It saves individual displacement components into an AuxVariable for visualization and/or post-processing purposes.


## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/global_strain/global_strain.i block=AuxKernels/disp_y

An AuxVariable is required to store the displacements calculated by the AuxKernel.
The name of the AuxVariable is used as the argument for the `variable` input parameter.

!listing modules/tensor_mechanics/test/tests/global_strain/global_strain.i block=AuxVariables/disp_y

## Visualization

 In order to visualize the deformed shape with respect to the displacements calculated here, name the AuxVariables as `disp_x, disp_y, disp_z` and use some other names for the displacement variables.


!syntax parameters /AuxKernels/GlobalDisplacementAux

!syntax inputs /AuxKernels/GlobalDisplacementAux

!syntax children /AuxKernels/GlobalDisplacementAux

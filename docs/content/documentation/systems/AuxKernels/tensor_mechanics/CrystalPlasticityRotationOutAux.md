# Crystal Plasticity Rotation Out Aux
!syntax description /AuxKernels/CrystalPlasticityRotationOutAux

## Description
This AuxKernel, `CrystalPlasticityRotationOutAux` is intended for use in crystal plasticity simulations with texture evolution of the crystal lattice and for debugging purposes to verify the Euler Angles used in a simulation.
Depending on the setting of the `output_frequency` parameter, this AuxKernel will save the values of the [Rotation matrix](http://mathworld.wolfram.com/EulerAngles.html) for the crystal by writing to a separate file.

Because this AuxKernel writes to a separate file as often as every timestep (the default value of the `output_frequency` parameter is 1), use of this AuxKernel can be computationally expensive.
Caution in using this AuxKernel is strongly recommended.

## Example Input File Syntax
!listing modules/tensor_mechanics/test/tests/cp_user_object/crysp_save_euler.i block=AuxKernels/rotout

An AuxVariable is required to store the `CrystalPlasticityRotationOutAux` AuxKernel information. Note that the name of the AuxVariable is used as the argument for the `variable` input parameter in the `CrystalPlasticityRotationOutAux` block.
!listing modules/tensor_mechanics/test/tests/cp_user_object/crysp_save_euler.i block=AuxVariables/rotout

!syntax parameters /AuxKernels/CrystalPlasticityRotationOutAux

!syntax inputs /AuxKernels/CrystalPlasticityRotationOutAux

!syntax children /AuxKernels/CrystalPlasticityRotationOutAux

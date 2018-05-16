# Global Strain Action

!syntax description /Modules/TensorMechanics/GlobalStrain/GlobalStrainAction

This action simplifies the input file syntax for global strain calculation required for maintaining strain periodicity. It also generates the auxiliary displacement field created by the global strain. It creates following MOOSE objects,

### UserObject

- [Global Strain UserObject](/GlobalStrainUserObject.md): Calculates the residual and jacobian corresponding to the scalar variable

### ScalarKernel

- [Global Strain ScalarKernel](/GlobalStrain.md): Solves for the global strain in terms of the scalar variable

### Material

- [Global Strain Material](/ComputeGlobalStrain.md): Calculates the global strain components from the scalar variable

### AuxVariables

- [Global Displacement AuxVariables](/GlobalDisplacementAux.md): Creates AuxVariables for each additional displacement components

### AuxKernels

- [Global Displacement AuxKernels](/GlobalDisplacementAux.md): Calculates the global displacement field from the scalar variable


!syntax parameters /Modules/TensorMechanics/GlobalStrain/GlobalStrainAction

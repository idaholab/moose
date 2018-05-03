# Nodal Stress Strain AuxKernel

## Description

AuxKernel `NodalStressStrainPD` is used to output the components of strain and stress tensors and their equivalent scalar quantities (for now only the von Mises stress) for bond-based and ordinary state-based models. And this is only applies to elastic simulation. For correspondence models, the UserObjects `NodalRankTwoTensorComponentPD` and `NodalRankTwoTensorScalarPD` should be used.

This is a stand-alone calculation based on the concept of deformation gradient in correspondence model for postprocessing. Given the deformation gradient at each material point, other rank two tensors (strain, stress) can be calculated using the relationships from continuum mechanics theory.

For stress calculation, material constants, i.e., Young's modulus and Poisson's ratio, are required as input parameters.

!syntax parameters /AuxKernels/NodalStressStrainPD

!syntax inputs /AuxKernels/NodalStressStrainPD

!syntax children /AuxKernels/NodalStressStrainPD

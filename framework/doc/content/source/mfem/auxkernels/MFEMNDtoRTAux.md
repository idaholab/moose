# MFEMNDtoRTAux

!if! function=hasCapability('mfem')

## Overview

Auxkernel for copying the DoF of a source variable defined on an $H(\mathrm{curl})$ conforming Nedelec (ND) finite element (FE) space into a target variable defined on an $H(\mathrm{div})$ conforming Raviart-Thomas (RT) FE space.

This auxkernel is intended ONLY for two-dimensional MFEM ND to RT FE spaces. In 2D MFEM, the ND to RT bases are related by a 90 degree rotation, so copying the ND coefficients into a compatible RT space can be used to represent the rotated field. 

This is useful in 2D magnetostatic postprocessing. After solving for the scalar field
$A_z \in H^1$, representing the out-of-plane component of the magnetic vector potential,
the gradient may be computed in an ND space using [MFEMGradAux.md]. The magnetic flux
density is then given by a 90-degree rotation of this gradient.

## Example Input File Syntax

!listing mfem/auxkernels/2Dmagnetostatic.i block=AuxKernels

!syntax parameters /AuxKernels/MFEMNDtoRTAux

!syntax inputs /AuxKernels/MFEMNDtoRTAux

!syntax children /AuxKernels/MFEMNDtoRTAux

!if-end!

!else
!include mfem/mfem_warning.md

# MFEMAxisymmetricCurlAthetaAux

!if! function=hasCapability('mfem')

## Overview

Auxkernel for computing the axisymmetric curl of an azimuthal scalar source variable $A_\theta$ and projecting the resulting field in a vector MFEM auxilary variable. 

In cylindrical coordinates $(r,\theta,z)$, the curl of a purely azimuthal and axisymmetric field, $\mathbf{A} = A_\theta(r,z)\mathbf{e}_\theta$, is 

!equation
\nabla \times \mathbf{A} = -\frac{\partial A_\theta}{\partial z}\mathbf{e}_r + \left( \frac{\partial A_\theta}{\partial r}+\frac{A_\theta}{r} \right)\mathbf{e}_z 

This auxkernel is useful, e.g., in axisymmetric electromagnetic calculations to compute the magnetic flux density $\mathbf{B} = \nabla \times \mathbf{A}$ from $A_\theta$, representing the azimuthal component of the magnetic vector potential $\mathbf{A}$.


## Example Input File Syntax

!listing mfem/auxkernels/axisymmetric_magnetostatic.i block=AuxKernels

!syntax parameters /AuxKernels/MFEMAxisymmetricCurlAthetaAux

!syntax inputs /AuxKernels/MFEMAxisymmetricCurlAthetaAux

!syntax children /AuxKernels/MFEMAxisymmetricCurlAthetaAux

!if-end!

!else
!include mfem/mfem_warning.md

# MFEMAxisymmetricCurlAthetaAux

!if! function=hasCapability('mfem')

## Overview

Auxkernel for computing the axisymmetric curl of an azimuthial scalar source variable $A_\theta$ and projecting the resulting field in a vector MFEM auxilary variable. 

For an axisymmetric field $\mathbf{A_\theta} = A_\theta(r,z)\mathbf{e}_\theta,$ in cylindrical coordinates $(r,\theta)$ with no dependence on the azimuthal coordinate the curl is 

!equation
\nabla \times \mathbf{A} = \frac{\partial A_\theta}{\partial z}\mathbf{e}_r + \left( \frac{\partial A_\theta}{\partial r}+\frac{A_\theta}{r} \right)\mathbf{e}_z 

This useful for axisymmetric electromagnetic calculations where $A_\theta$ represents the azimuthal component of the magnetic vector potential. Therefore $\nabla \times \mathbf{A}$ corresponds to 
to the magnetic flux density $\mathbf{B} = \nabla \times \mathbf{A}$ in the $(r,z)$.


## Example Input File Syntax

!listing mfem/auxkernels/axisymmetric_magnetostatic.i block=AuxKernels

!syntax parameters /AuxKernels/MFEMAxisymmetricCurlAthetaAux

!syntax inputs /AuxKernels/MFEMAxisymmetricCurlAthetaAux

!syntax children /AuxKernels/MFEMAxisymmetricCurlAthetaAux

!if-end!

!else
!include mfem/mfem_warning.md

# MFEMMeridionalCurlAux

!if! function=hasCapability('mfem')

## Overview

Auxkernel for computing the meridional curl of an azimuthal scalar source variable $A_\theta$ and projecting the resulting field in a vector MFEM auxiliary variable. This AuxKernel is intended for a 2D meridional $(r,z)$ formulation represented on a Cartesian mesh with coordinate convetion

!equation
x=r, \qquad y=z

The first mesh coordinate is intepreted as the cylindrical radius $r$, and the second mesh coordinate as the axial coordinate. This AuxKernel therefore uses

!equation
r = x, \qquad \frac{1}{r} = \frac{1}{x}

and calculates the curl as 

!equation
\nabla \times \mathbf{A} = -\frac{\partial A_\theta}{\partial z}\mathbf{e}_r + \left( \frac{\partial A_\theta}{\partial r}+\frac{A_\theta}{r} \right)\mathbf{e}_z 

This auxkernel is useful, e.g., in axisymmetric electromagnetic calculations to compute the magnetic flux density $\mathbf{B} = \nabla \times \mathbf{A}$ from $A_\theta$, representing the azimuthal component of the magnetic vector potential $\mathbf{A}$.


## Example Input File Syntax

!listing mfem/auxkernels/axisymmetric_magnetostatic.i block=AuxKernels

!syntax parameters /AuxKernels/MFEMMeridionalCurlAux

!syntax inputs /AuxKernels/MFEMMeridionalCurlAux

!syntax children /AuxKernels/MFEMMeridionalCurlAux

!if-end!

!else
!include mfem/mfem_warning.md

# MFEMMeridionalCurlAux

!if! function=hasCapability('mfem')

## Overview

In cylindrical coordinates $(r,\theta,z)$, the curl of a purely azimuthal and
axisymmetric field, $\mathbf{A} = A_\theta(r,z)\mathbf{e}_\theta$, is

!equation
\nabla \times \mathbf{A} = -\frac{\partial A_\theta}{\partial z}\mathbf{e}_r + \left( \frac{\partial A_\theta}{\partial r}+\frac{A_\theta}{r} \right)\mathbf{e}_z

This auxkernel computes this quantity for a 2D meridional $(r,z)$ formulation
represented on a Cartesian mesh with coordinate convention $(r=x, z=y)$
and projects the resulting field in a vector MFEM auxiliary variable.

This auxkernel is useful, e.g., in axisymmetric electromagnetic calculations to compute the magnetic flux density $\mathbf{B} = \nabla \times \mathbf{A}$ from $A_\theta$, representing the azimuthal component of the magnetic vector potential $\mathbf{A}$.


## Example Input File Syntax

!listing mfem/auxkernels/axisymmetric_magnetostatic.i block=AuxKernels

!syntax parameters /AuxKernels/MFEMMeridionalCurlAux

!syntax inputs /AuxKernels/MFEMMeridionalCurlAux

!syntax children /AuxKernels/MFEMMeridionalCurlAux

!if-end!

!else
!include mfem/mfem_warning.md

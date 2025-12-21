# MFEMComplexDivAux

!if! function=hasCapability('mfem')

## Overview

AuxKernel for calculating the divegence of a complex $H(\mathrm{div})$ conforming source variable defined
on a 3D Raviart-Thomas finite element space and storing it in a complex scalar elemental result variable
defined on an $L^2$ FE space.

The result may be scaled by an optional (global) complex scalar factor.

!equation
v =  \lambda \vec\nabla \cdot \vec u

where $\vec u \in H(\mathrm{div})$, $v \in L^2$ and $\lambda$ is a complex scalar constant.

## Input File Syntax

!syntax parameters /AuxKernels/MFEMComplexDivAux

!syntax inputs /AuxKernels/MFEMComplexDivAux

!syntax children /AuxKernels/MFEMComplexDivAux

!if-end!

!else
!include mfem/mfem_warning.md

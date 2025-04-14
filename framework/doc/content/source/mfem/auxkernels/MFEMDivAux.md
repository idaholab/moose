# MFEMDivAux

!if! function=hasCapability('mfem')

## Summary

!syntax description /AuxKernels/MFEMDivAux

## Overview

AuxKernel for calculating the divegence of an $H(\mathrm{div})$ conforming source variable defined
on a 3D Raviart-Thomas finite element space and storing it in a scalar elemental result variable
defined on an $L^2$ FE space.

The result may be scaled by an optional (global) scalar factor.

!equation
v =  \lambda \vec\nabla \cdot \vec u

where $\vec u \in H(\mathrm{div})$, $v \in L^2$ and $\lambda$ is a scalar constant.

## Example Input File Syntax

!listing graddiv.i

!syntax parameters /AuxKernels/MFEMDivAux

!syntax inputs /AuxKernels/MFEMDivAux

!syntax children /AuxKernels/MFEMDivAux

!if-end!

!else
!include mfem/mfem_warning.md

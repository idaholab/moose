# MFEMScalarTimeAverageAux

!if! function=hasCapability('mfem')

## Summary

!syntax description /AuxKernels/MFEMScalarTimeAverageAux

## Overview

AuxKernel for calculating the running time average of a scalar coefficient
during a transient simulation using the rectangle rule.

!equation
v(t) = (1 - \omega) v(t - \Delta t) + \omega u(t), \,\,\, \omega = \Delta t / (t - s), \,\,\, t > s

where $u$ is the scalar coefficient to take the average of, $v \in H^1$ or $L^2$ is the
scalar variable holding the time average of $u$, and $s$ is a timeskip, i.e. a timespan at the beginning
of the simulation, prescribed by the user, during which no time averaging occurs.

## Example Input File Syntax

!listing mfem/kernels/heattransfer.i block=/AuxKernels

!syntax parameters /AuxKernels/MFEMScalarTimeAverageAux

!syntax inputs /AuxKernels/MFEMScalarTimeAverageAux

!syntax children /AuxKernels/MFEMScalarTimeAverageAux

!if-end!

!else
!include mfem/mfem_warning.md

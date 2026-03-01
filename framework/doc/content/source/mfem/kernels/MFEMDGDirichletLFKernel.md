# MFEMDGDirichletLFKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the linear form

!equation
(f, v)_\Omega \,\,\, \forall v \in V

where $v \in H^1$ is the test variable and $f$ is a
scalar forcing coefficient.

This term arises from the weak form of the forcing term

!equation
f

!if-end!

!else
!include mfem/mfem_warning.md

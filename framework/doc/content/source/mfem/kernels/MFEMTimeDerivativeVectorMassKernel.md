# MFEMTimeDerivativeVectorMassKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the bilinear form

!equation
(k \dot{\vec u}, \vec v)_\Omega \,\,\, \forall \vec v \in V

where $\vec u$ and $\vec v$ belong to vector-valued $H^1$ or $L^2$ spaces and $k$ is a scalar
coefficient. If `trial_variable` is set to a variable other than the test variable, the time
derivative of that variable is coupled into the equation for the test variable. This allows second
order in time problems, such as elastodynamics, to be written as a first order system: the example
below solves for a displacement $\vec u$ and velocity $\vec w$ with

!equation
(\rho \dot{\vec w}, \vec v)_\Omega + (\sigma(\vec u), \nabla \vec v)_\Omega = (\vec f, \vec v)_\Omega,
\qquad (\dot{\vec u}, \vec z)_\Omega - (\vec w, \vec z)_\Omega = 0.

This term arises from the weak form of the operator

!equation
k \dot{\vec u}

## Example Input File Syntax

!listing mfem/kernels/elastodynamics.i block=/Kernels

!syntax parameters /Kernels/MFEMTimeDerivativeVectorMassKernel

!syntax inputs /Kernels/MFEMTimeDerivativeVectorMassKernel

!syntax children /Kernels/MFEMTimeDerivativeVectorMassKernel

!if-end!

!else
!include mfem/mfem_warning.md

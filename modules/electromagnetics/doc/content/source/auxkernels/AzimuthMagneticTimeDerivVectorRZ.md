# AzimuthMagneticTimeDerivVectorRZ

!syntax description /AuxKernels/AzimuthMagneticTimeDerivVectorRZ

## Overview

!style halign=left
The AzimuthMagneticTimeDerivVectorRZ object calculates the time derivative of the azimuthal magnetic field for an axisymmetric cylindrical configuration. This object evaluates the right hand side of the following equation:

\begin{equation}
  \frac{\partial \vec{B}_{\theta}}{\partial t} = -\left( \nabla \times \vec{E} \right)_{\theta}
\end{equation}

where

- $\vec{B}$ is the magnetic field,
- $\vec{E}$ is the electric field, and
- the subscript $\theta$ is the azimuthal component of the field.

## Example Input File Syntax

!listing vector_azim_magnetic_time_deriv.i block=AuxKernels/aux_azim_dB_dt_vector

!syntax parameters /AuxKernels/AzimuthMagneticTimeDerivVectorRZ

!syntax inputs /AuxKernels/AzimuthMagneticTimeDerivVectorRZ

!syntax children /AuxKernels/AzimuthMagneticTimeDerivVectorRZ

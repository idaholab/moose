# AzimuthMagneticTimeDerivScalar

!syntax description /AuxKernels/AzimuthMagneticTimeDerivScalar

## Overview

!style halign=left
The AzimuthMagneticTimeDerivScalar object calculates the time derivative of the azimuthal magnetic field for an axisymmetric cylindrical configuration. The term is define as:

\begin{equation}
  \frac{\partial \vec{B}_{\theta}}{\partial t} = -\left( \frac{\partial \vec{E}_{\rho}}{\partial z} - \frac{\partial \vec{E}_{z}}{\partial \rho} \right)
\end{equation}

where

- $\vec{B}_{\theta}$ is the azimuthal component of the magnetic field,
- $\vec{E}_{\rho}$ is the radial component of the electric field, and
- $\vec{E}_{z}$ is the axial component of the electric field.

## Example Input File Syntax

!listing scalar_azim_magnetic_time_deriv.i block=AuxKernels/aux_azim_dB_dt_scalar

!syntax parameters /AuxKernels/AzimuthMagneticTimeDerivScalar

!syntax inputs /AuxKernels/AzimuthMagneticTimeDerivScalar

!syntax children /AuxKernels/AzimuthMagneticTimeDerivScalar

# AzimuthMagneticTimeDerivRZ

!syntax description /AuxKernels/AzimuthMagneticTimeDerivRZ

## Overview

!style halign=left
The AzimuthMagneticTimeDerivRZ object calculates the time derivative of the azimuthal magnetic field for an axisymmetric cylindrical configuration. 
The electric field can be supplied as a vector or scalar components.

If the electric field is provided as a vector, this object evaluates the right hand side of the following equation:

\begin{equation}
  \frac{\partial \vec{B}_{\theta}}{\partial t} = -\left( \nabla \times \vec{E} \right)_{\theta}
\end{equation}

where

- $\vec{B}$ is the magnetic field,
- $\vec{E}$ is the electric field, and
- the subscript $\theta$ is the azimuthal component of the field.

If the electric field is provided as scalar components, this object evaluates the right hand side of the following equation:

\begin{equation}
  \frac{\partial \vec{B}_{\theta}}{\partial t} = -\left( \frac{\partial \vec{E}_{\rho}}{\partial z} - \frac{\partial \vec{E}_{z}}{\partial \rho} \right)
\end{equation}

where

- $\vec{E}_{\rho}$ is the radial component of the electric field, and
- $\vec{E}_{z}$ is the axial component of the electric field.

## Example Input File Syntax

The following is an example when suppling the electric field as a vector:

!listing vector_azim_magnetic_time_deriv.i block=AuxKernels/aux_azim_dB_dt_vector

The following is an example when suppling the electric field as scalar components:

!listing scalar_azim_magnetic_time_deriv.i block=AuxKernels/aux_azim_dB_dt_scalar

!syntax parameters /AuxKernels/AzimuthMagneticTimeDerivRZ

!syntax inputs /AuxKernels/AzimuthMagneticTimeDerivRZ

!syntax children /AuxKernels/AzimuthMagneticTimeDerivRZ

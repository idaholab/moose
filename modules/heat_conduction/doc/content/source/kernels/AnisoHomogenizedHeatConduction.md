# AnisoHomogenizedHeatConduction

!syntax description /Kernels/AnisoHomogenizedHeatConduction

!alert note title=Einstein summation convention
Einstein summation convention is used in this documentation page.

## Description

This `Kernel` computes the right hand side of the equation

!equation
\int_Y \frac{\partial v}{\partial y_i}\lambda_{ij} \frac{\partial \chi^k}{\partial y_j} = -\int_Y \frac{\partial v}{\partial y_i}\lambda_{ik} \text{d}y

where $\lambda_{ik}$ is the thermal conductivity tensor, $y_i$ is the coordinate in the unit cell, and $\chi^k$ is the $k$-th characteristic function used for homogenizing the thermal conductivity.  It is used in conjunction with the [Anisotropic Heat Conduction](AnisoHeatConduction.md) `Kernel` and the [Homogenized Thermal Conductivity](HomogenizedThermalConductivity.md) `Postprocessor` to compute homogenized thermal conductivity values.

This homogenization is executed for a unit cell with periodic boundary conditions. For
any vector $\vec{y}_b$ on the boundary, the unit cell geometry must satisfy the condition:

!equation
\vec{n}(\vec{y}_b + \vec{p}) = -\vec{n}(\vec{y}_b),

where $\vec{n}$ is the outward normal vector, and $\vec{p}$ is the periodicity of the boundary where $\vec{y}_b$ is located.

This kernel is the anisotropic version of [HomogenizedHeatConduction](HomogenizedHeatConduction.md).
See [!cite](hales15homogenization) and [!cite](SONG2006710) for more details.

## Example Input File Syntax

!listing modules/heat_conduction/test/tests/homogenization/heatConduction2D_tensor_tc.i block=Kernels/heat_rhs_x

!syntax parameters /Kernels/AnisoHomogenizedHeatConduction

!syntax inputs /Kernels/AnisoHomogenizedHeatConduction

!syntax children /Kernels/AnisoHomogenizedHeatConduction

!bibtex bibliography

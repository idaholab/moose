# HomogenizedHeatConduction

!syntax description /Kernels/HomogenizedHeatConduction

!alert note title=Einstein summation convention
Einstein summation convention is used in this documentation page.

## Description

This `Kernel` computes the right hand side of the equation of the equation

!equation
\int_Y \frac{\partial v}{\partial y_i}\lambda_{ij} \frac{\partial \chi^k}{\partial y_j} = -\int_Y \frac{\partial v}{\partial y_i}\lambda_{ik} \text{d}y

where $\lambda_{ik}$ is the thermal conductivity tensor, $y_i$ is the coordinate in the unit cell, and $\chi^k$ is the $k$-th characteristic function used for homogenizing the thermal conductivity.  
In this case, the thermal conductivity is isotropic, i.e. $\lambda_{ik} = \lambda \delta_{ij}$ with $k$ being the thermal conductivity and $\delta_{ij}$ being the Kronecker delta.
The equation simplifies to:

!equation
\int_Y \frac{\partial v}{\partial y_i}\lambda \frac{\partial \chi^k}{\partial y_j} = -\int_Y \frac{\partial v}{\partial y_k}\lambda \text{d}y

This kernel is used in conjunction with the [Heat Conduction](HeatConduction.md) `Kernel` and the [Homogenized Thermal Conductivity](HomogenizedThermalConductivity.md) `Postprocessor` to compute homogenized thermal conductivity values.

This homogenization is executed for a unit cell with periodic boundary conditions. For
any vector $\vec{y}_b$ on the boundary, the unit cell geometry must satisfy the condition:

!equation
\vec{n}(\vec{y}_b + \vec{p}) = -\vec{n}(\vec{y}_b),

where $\vec{n}$ is the outward normal vector, and $\vec{p}$ is the periodicity of the boundary where $\vec{y}_b$ is located.

See [!cite](hales15homogenization) and [!cite](SONG2006710) for more details.

## Example Input File Syntax

!listing modules/heat_conduction/test/tests/homogenization/heatConduction2D.i block=Kernels/heat_rhs_x

!syntax parameters /Kernels/HomogenizedHeatConduction

!syntax inputs /Kernels/HomogenizedHeatConduction

!syntax children /Kernels/HomogenizedHeatConduction

!bibtex bibliography

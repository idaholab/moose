# HomogenizedHeatConduction

!syntax description /Kernels/HomogenizedHeatConduction

## Description

This `Kernel` computes the right hand side of the equation of the equation

!equation
\int_Y \frac{\partial v}{\partial y_i}k_{ij} \frac{\partial \chi^k}{\partial y_j} = -\int_Y \frac{\partial v}{\partial y_i}k_{ik} \text{d}y

where $k_{ik}$ is the thermal conductivity and $\chi^k$ is a function used for homogenizing the thermal conductivity.  
In this case, the thermal conductivity is isotropic, i.e. $k_{ik} = k \delta_{ij}$ with $k$ being the thermal conductivity and $\delta_{ij}$ being the Kronecker delta.
The equation simplifies to:

!equation
\int_Y \frac{\partial v}{\partial y_i}k \frac{\partial \chi^k}{\partial y_j} = -\int_Y \frac{\partial v}{\partial y_k}k \text{d}y

This kernel is used in conjunction with the [Heat Conduction](HeatConduction.md) `Kernel` and the [Homogenized Thermal Conductivity](HomogenizedThermalConductivity.md) `Postprocessor` to compute homogenized thermal conductivity values.  
See [!cite](hales15homogenization) and [!cite](SONG2006710) for more details.

## Example Input File Syntax

!listing modules/heat_conduction/test/tests/homogenization/heatConduction2D.i block=Kernels/heat_rhs_x

!syntax parameters /Kernels/HomogenizedHeatConduction

!syntax inputs /Kernels/HomogenizedHeatConduction

!syntax children /Kernels/HomogenizedHeatConduction

!bibtex bibliography

# AnisoHomogenizedHeatConduction

!syntax description /Kernels/AnisoHomogenizedHeatConduction

## Description

This `Kernel` computes the right hand side of the equation

!equation
\int_Y \frac{\partial v}{\partial y_i}k_{ij} \frac{\partial \chi^k}{\partial y_j} = -\int_Y \frac{\partial v}{\partial y_i}k_{ik} \text{d}y

where $k_{ik}$ is the thermal conductivity tensor and $\chi^k$ is a function used for homogenizing the thermal conductivity.  It is used in conjunction with the [Anisotropic Heat Conduction](AnisoHeatConduction.md) `Kernel` and the [Homogenized Thermal Conductivity](HomogenizedThermalConductivity.md) `Postprocessor` to compute homogenized thermal conductivity values.
This kernel is the anisotropic version of [HomogenizedHeatConduction](HomogenizedHeatConduction.md).
See [!cite](hales15homogenization) and [!cite](SONG2006710) for more details.

## Example Input File Syntax

!listing modules/heat_conduction/test/tests/homogenization/heatConduction2D_tensor_tc.i block=Kernels/heat_rhs_x

!syntax parameters /Kernels/AnisoHomogenizedHeatConduction

!syntax inputs /Kernels/AnisoHomogenizedHeatConduction

!syntax children /Kernels/AnisoHomogenizedHeatConduction

!bibtex bibliography

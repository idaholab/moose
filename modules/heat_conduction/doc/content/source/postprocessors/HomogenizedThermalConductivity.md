# HomogenizedThermalConductivity

!syntax description /Postprocessors/HomogenizedThermalConductivity

## Description

This `PostProcessor` computes
\begin{equation}
k_{ij}^\text{H}=\frac{1}{\left|\text{Y}\right|}\int_\text{Y}k_{ij}\left(\bm{I} + \frac{\partial\psi^k}{\partial y_j}\right)\;\text{d}\bm{y}
\end{equation}
where $k^\text{H}_{ij}$ is the homogenized thermal conductivity.  It is used in conjunction with the [Heat Conduction](HeatConduction.md) `Kernel` and the [Homogenized Heat Conduction](HomogenizedHeatConduction.md) `Kernel` to compute homogenized thermal conductivity values according to
\begin{equation}
\int_Y \frac{\partial v}{\partial y_i}k_{ik} \frac{\partial \psi^k}{\partial y_j} = \int_Y \frac{\partial v}{\partial y_i}k_{ik} \text{d}y
\end{equation}
where $k$ is the diffusion coefficient (thermal conductivity).  See [!cite](hales15homogenization).

## Example Input File Syntax

!listing modules/heat_conduction/test/tests/homogenization/heatConduction2D.i block=Postprocessors/k_xx


!syntax parameters /Postprocessors/HomogenizedThermalConductivity

!syntax inputs /Postprocessors/HomogenizedThermalConductivity

!syntax children /Postprocessors/HomogenizedThermalConductivity

!bibtex bibliography

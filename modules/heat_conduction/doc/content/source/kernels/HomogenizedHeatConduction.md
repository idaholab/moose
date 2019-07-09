# HomogenizedHeatConduction

!syntax description /Kernels/HomogenizedHeatConduction

## Description

This `Kernel` computes
\begin{equation}
\int_Y \frac{\partial v}{\partial y_i}k_{ik} \textrm{d}y
\end{equation}
where $k$ is the diffusion coefficient (thermal conductivity).  It is used in conjunction with the [Heat Conduction](HeatConduction.md) `Kernel` and the [Homogenized Thermal Conductivity](HomogenizedThermalConductivity.md) `PostProcessor` to compute homogenized thermal conductivity values according to
\begin{equation}
\int_Y \frac{\partial v}{\partial y_i}k_{ik} \frac{\partial \psi^k}{\partial y_j} = \int_Y \frac{\partial v}{\partial y_i}k_{ik} \text{d}y
\end{equation}
and
\begin{equation}
k_{ij}^\text{H}=\frac{1}{\left|\text{Y}\right|}\int_\text{Y}k_{ij}\left(\bm{I} + \frac{\partial\psi^k}{\partial y_j}\right)\;\text{d}\bm{y}
\end{equation}
where $k^H_{ij}$ is the homogenized thermal conductivity.  See [cite](hales15homogenization).

## Example Input File Syntax

!listing modules/combined/test/tests/homogenization/heatConduction2D.i block=Kernels/heat_rhs_x

!syntax parameters /Kernels/HomogenizedHeatConduction

!syntax inputs /Kernels/HomogenizedHeatConduction

!syntax children /Kernels/HomogenizedHeatConduction

!bibtex bibliography

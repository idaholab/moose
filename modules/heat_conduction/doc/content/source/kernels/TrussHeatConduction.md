# TrussHeatConduction

!syntax description /Kernels/TrussHeatConduction

This kernel provides the conduction term in the heat equation for truss elements, where the cross-sectional area of the truss should be taken into account. It is similar to the standard [HeatConduction](HeatConduction.md) kernel, but multiplies the contribution by a cross-sectional area, which is provided using a coupled variable:

\begin{equation}
\nabla \cdot (k A \nabla T)
\end{equation}
where A is cross-sectional area of the truss element, $k$ is thermal conduction of material.

This kernel should only be used for 1D line elements, which can be used to model heat conduction through networks of trusses in 1D, 2D, or 3D space. It is important to note that this should not be used for 1D models in 1D space if the intent is to model heat conduction through an infinite medium (in 1D Cartesian coordinates) or through an axisymmetric volume. The standard HeatConduction kernel should be used in those situations.

!syntax parameters /Kernels/TrussHeatConduction

!syntax inputs /Kernels/TrussHeatConduction

!syntax children /Kernels/TrussHeatConduction

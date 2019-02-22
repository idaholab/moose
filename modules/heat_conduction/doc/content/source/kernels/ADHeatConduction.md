# ADHeatConduction

## Description

`ADHeatConduction` is the implementation of the heat diffusion equation in [HeatConduction](/HeatConduction.md) within the framework of automatic differentiation.
The `ADHeatConduction` kernel implements the heat equation given by Fourier's Law where The heat flux is given as
\begin{equation}
\mathbf{q} = - k\nabla T,
\end{equation}
where $k$ denotes the thermal conductivity of the material. $k$ can either be an `ADMaterial` or traditional `Material`.

This class inherits from the [ADDiffusion](/ADDiffusion.md) class.

!syntax description /Kernels/ADHeatConduction<RESIDUAL>

!syntax parameters /Kernels/ADHeatConduction<RESIDUAL>

!syntax inputs /Kernels/ADHeatConduction<RESIDUAL>

!syntax children /Kernels/ADHeatConduction<RESIDUAL>

!bibtex bibliography

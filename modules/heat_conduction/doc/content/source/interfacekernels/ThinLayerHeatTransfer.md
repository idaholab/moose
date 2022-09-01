# ThinLayerHeatTransfer

!syntax description /InterfaceKernels/ThinLayerHeatTransfer

## Description

This kernel models the heat transfer across a thin domain. The purpose is to model a thin domain with an interface. This will reduce number of mesh elements and avoid high aspect ratio elements within a thin domain. The flux for each side of the gap are defined by the following equations:

\begin{equation}
-n_+\cdot q_+ = \frac{dQ}{2}-\rho C_p(\frac{d}{2}\frac{\partial T_+}{\partial t}) - (-k\frac{T_- -T_+}{d})
\end{equation}
\begin{equation}
-n_-\cdot q_- = \frac{dQ}{2}-\rho C_p(\frac{d}{2}\frac{\partial T_-}{\partial t}) - (-k\frac{T_+ -T_-}{d})
\end{equation}
where the $+$ and $-$ indices indicate the primary and neighbor side of the boundary, respectively, $d$ is the layer thickness, $Q$ is a heat source in the layer and $k$, $C_p$ and $\rho$ are thermal conductivity, specific heat and density of the layer.

## Example Input File Syntax

!listing modules/heat_conduction/test/tests/thin_layer_heat_transfer/transient_2d.i
 block=InterfaceKernels

!syntax parameters /InterfaceKernels/ThinLayerHeatTransfer

!syntax inputs /InterfaceKernels/ThinLayerHeatTransfer

!syntax children /InterfaceKernels/ThinLayerHeatTransfer

!bibtex bibliography

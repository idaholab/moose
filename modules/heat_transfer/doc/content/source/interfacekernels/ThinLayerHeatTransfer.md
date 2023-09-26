# ThinLayerHeatTransfer

!syntax description /InterfaceKernels/ThinLayerHeatTransfer

## Description

This kernel models the heat transfer across a thin domain. The purpose is to model a thin domain with an interface. This will reduce number of mesh elements and avoid high aspect ratio elements within a thin domain. The flux for each side of the gap are defined by the following equations:

\begin{equation}
-n^+\cdot q^+ = \frac{dQ}{2}-\rho C_p(\frac{d}{2}\frac{\partial T^+}{\partial t}) - (-k\frac{T^- -T^+}{d})
\end{equation}
\begin{equation}
-n^-\cdot q^- = \frac{dQ}{2}-\rho C_p(\frac{d}{2}\frac{\partial T^-}{\partial t}) - (-k\frac{T^+ -T^-}{d})
\end{equation}
where the $+$ and $-$ indices indicate the primary and neighbor side of the boundary, respectively, $d$ is the layer thickness, $Q$ is a heat source in the layer and $k$, $C_p$ and $\rho$ are thermal conductivity, specific heat and density of the layer.

An example below is used to verify the thin layer heat transfer model. With the interface approach, heat transfer in the thin layer is solved at the interface. Its solution is compared against the case where a thin domain is explicitly represented in the finite element domain.

!media media/heat_conduction/two_bl_mesh.png
       style=width:650px;margin-left:70px;float:center;
       id=fig:two_bl_mesh
       caption=Finite element mesh used for thin layer heat transfer using interfacekernel.

!media media/heat_conduction/two_bl_temp.png
       style=width:650px;margin-left:70px;float:center;
       id=fig:two_bl_temp
       caption=Temperature field for thin layer heat transfer using interfacekernel.

!media media/heat_conduction/three_bl_mesh.png
       style=width:650px;margin-left:70px;float:center;
       id=fig:three_bl_mesh
       caption=Finite element mesh used for thin layer heat transfer using a thin domain.

!media media/heat_conduction/three_bl_temp.png
       style=width:650px;margin-left:70px;float:center;
       id=fig:three_bl_temp
       caption=Temperature field for thin layer heat transfer using a thin domain.

## Example Input File Syntax

!listing modules/heat_conduction/test/tests/thin_layer_heat_transfer/transient_2d.i
 block=InterfaceKernels

!syntax parameters /InterfaceKernels/ThinLayerHeatTransfer

!syntax inputs /InterfaceKernels/ThinLayerHeatTransfer

!syntax children /InterfaceKernels/ThinLayerHeatTransfer

!bibtex bibliography

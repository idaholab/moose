# SideSetHeatTransferKernel

!syntax description /InterfaceKernels/SideSetHeatTransferKernel

## Description

This kernel models the heat transfer across a side set using all three heat transfer mechanisms. The purpose is to model the heat transfer across a small gap without modeling the gap itself. The heat transfer mechanisms for each side of the gap are described by the following equations:
\begin{equation}
  q''^{+}_{\mathrm{conduction}} = q''^{-}_{\mathrm{conduction}} = -C_{\mathrm{gap}} (T_{+}-T_{-}),
\end{equation}
\begin{equation}
  q''^{\pm}_{\mathrm{convection}} = \pm h_{\mathrm{gap}}^{\pm} (T_{\mathrm{bulk}} - T_{\pm}),
\end{equation}
\begin{equation}
  q''^+_{\mathrm{radiation}} = q''^-_{\mathrm{radiation}} = \epsilon^{-}_{\mathrm{eff}}T_{-}^4 -  \epsilon^{+}_{\mathrm{eff}}T_{+}^4,
\end{equation}
where the $+$ and $-$ indices indicate the primary and neighbor side of the boundary, respectively.

 - $C_{\mathrm{gap}}$ is the gap conductance typically defined as conductivity divided by gap width: `conductance`

 - $h_{\mathrm{gap}}^{\pm}$ is the convective heat transfer coefficient: `h_primary`/`h_neighbor`

 - $T_{\mathrm{bulk}}$ is the bulk temperature of the gap, either defined by a material property or variable: `Tbulk_mat` or `Tbulk_var`

 - $\epsilon^{\pm}_{\mathrm{eff}}$ is the effective emmissivity: `emissivity_eff_primary`/`emissivity_eff_neighbor`

\begin{equation}
  \epsilon^{\pm}_{\mathrm{eff}} = \sigma\epsilon^{\pm}\frac{1-\rho^{\mp}}{1-\rho^{+}\rho^{-}}
\end{equation}

## Example Input File Syntax

Using material properties generated from [SideSetHeatTransferMaterial](SideSetHeatTransferMaterial.md):

!listing modules/heat_conduction/test/tests/sideset_heat_transfer/gap_thermal_1D.i block=InterfaceKernels/gap

Using bulk gap temperature (for convection) as auxiliary variable:

!listing modules/heat_conduction/test/tests/sideset_heat_transfer/gap_thermal_1D.i block=InterfaceKernels/gap_var

!syntax parameters /InterfaceKernels/SideSetHeatTransferKernel

!syntax inputs /InterfaceKernels/SideSetHeatTransferKernel

!syntax children /InterfaceKernels/SideSetHeatTransferKernel

!bibtex bibliography

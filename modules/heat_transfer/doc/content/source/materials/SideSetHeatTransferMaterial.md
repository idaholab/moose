# SideSetHeatTransferMaterial

!syntax description /Materials/SideSetHeatTransferMaterial

## Description

This is a interface material (boudary restricted material) specifically for application to [SideSetHeatTransferKernel](SideSetHeatTransferKernel.md). Material properties generated:

  - `gap_conductance`: $C_{\mathrm{gap}} = k_{\mathrm{gap}} / \delta$

  - `gap_Tbulk` : $T_{\mathrm{bulk}}$

  - `gap_h_primary` : $h^{+}_{\mathrm{gap}}$

  - `gap_h_neighbor` : $h^{-}_{\mathrm{gap}}$

  - `gap_emissivity_eff_primary` :

\begin{equation}
  \epsilon^{+}_{\mathrm{eff}} = \sigma\epsilon^{+}\frac{1-\rho^{-}}{1-\rho^{+}\rho^{-}}
\end{equation}

  - `gap_emissivity_eff_neighbor` :

\begin{equation}
  \epsilon^{-}_{\mathrm{eff}} = \sigma\epsilon^{-}\frac{1-\rho^{+}}{1-\rho^{+}\rho^{-}}
\end{equation}

$\sigma$ is the Stefan-Boltzmann constant defined as $5.670374419\times 10^{-8} \ W/m^2/K^4$.
The input variables that define these materials with their default values:

  - `conductivity`: $k(\vec{r},t)$ default: 0

  - `conductivity_temperature_function`: $k_{\mathrm{gap}}(\vec{r},T)$

  - `gap_temperature` : $T$ for $k_{\mathrm{gap}}(\vec{r},T)$

  - `gap_length` : $\delta$ default: 1

  - `Tbulk` : $T_{\mathrm{bulk}}$ default: 300 K

  - `h_primary` : $h^{+}_{\mathrm{gap}}$ default: 0

  - `h_neighbor` : $h^{-}_{\mathrm{gap}}$ default: 0

  - `emissivity_primary` : $\epsilon^{+}$ default: 0

  - `emissivity_neighbor` : $\epsilon^{-}$ default: 0

  - `reflectivity_primary` : $\rho^{+}$ default: $1-\epsilon^{+}$

  - `reflectivity_neighbor` : $\rho^{-}$ default: $1-\epsilon^{-}$


## Example Input File Syntax

Defining conductivity as a space-time dependent function:

!listing modules/heat_conduction/test/tests/sideset_heat_transfer/gap_thermal_1D.i block=Materials/gap_mat

Defining conductivity as a temperature dependent function:

!listing modules/heat_conduction/test/tests/sideset_heat_transfer/gap_thermal_ktemp_1D.i block=Materials/gap_mat

!syntax parameters /Materials/SideSetHeatTransferMaterial

!syntax inputs /Materials/SideSetHeatTransferMaterial

!syntax children /Materials/SideSetHeatTransferMaterial

!bibtex bibliography

# PorousFlowHeatEnergy

!syntax description /Postprocessors/PorousFlowHeatEnergy

This `Postprocessor` calculates the heat energy of fluid phase(s) $\beta$ using
\begin{equation*}
\mathcal{E} = \phi\sum_{\beta}S_{\beta}\rho_{\beta}\mathcal{E}_{\beta},
\end{equation*}
where all variables are defined in [`nomenclature`](/nomenclature.md).

The phases that the heat energy is summed over can be entered in the `phase` input
parameter. Multiple indices can be entered.

By default, the additional heat energy due to the porous material
\begin{equation*}
(1-\phi)\rho_{R}C_{R}T
\end{equation*}
is added to the heat energy of the fluid phase(s). This contribution can be ignored
by setting `include_porous_skeleton = false`.

!syntax parameters /Postprocessors/PorousFlowHeatEnergy

!syntax inputs /Postprocessors/PorousFlowHeatEnergy

!syntax children /Postprocessors/PorousFlowHeatEnergy

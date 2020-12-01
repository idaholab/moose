# PorousFlowHystereticRelativePermeabilityGas

This Material computes the gas relative permeability using the van Genuchten formulation.  The relative permeability is assumed to be hysteretic.  This Material may be used in two-phase situations only.

Along the drying curve, the relative permeability is
\begin{equation}
k_{r, g} = k_{r, g}^{max}\left(1 - \bar{S}_{l}\right)^{\gamma} \left( 1- \bar{S}_{l}^{1/m} \right)^{2m}
\end{equation}
where
\begin{equation}
\bar{S}_{l} = \frac{S_{l} - S_{l, r}}{1 - S_{l, r}}
\end{equation}
In these formulae:

- $S_{l}$ is the liquid saturation
- $S_{l, r}$ is the liquid residual saturation
- $k_{r, g}^{max}$ is the value of the gas relative permeability for $S_{l} \leq S_{l, r}$
- $m$ is the van Genuchten exponent
- $\gamma$ is another exponent

See the [hysteresis page](hysteresis.md) for more details concerning extending the formulation in the low-saturation region ($S_{l} < S_{l, r}$) and for a definition of the wetting relative permeability.

To use this Material, it is necessary to include a [PorousFlowHysteresisOrder](PorousFlowHysteresisOrder.md) Material, which computes and records information regarding the hysteresis order and the saturation turning points.

!syntax parameters /Materials/PorousFlowHystereticRelativePermeabilityGas

!syntax inputs /Materials/PorousFlowHystereticRelativePermeabilityGas

!syntax children /Materials/PorousFlowHystereticRelativePermeabilityGas

# PorousFlowHystereticRelativePermeabilityLiquid

This Material computes the liquid relative permeability using the van Genuchten formulation.  The relative permeability is assumed to be hysteretic.  This Material may be used in single-phase or two-phase situations only.

Along the drying curve, the relative permeability is
\begin{equation}
k_{r, l} = \sqrt{\bar{S}_{l}}\left[ 1 - \left( 1- \bar{S}_{l}^{1/m} \right)^{m} \right]^{2}
\end{equation}
where
\begin{equation}
\bar{S}_{l} = \frac{S_{l} - S_{l, r}}{1 - S_{l, r}}
\end{equation}
In these formulae:

- $S_{l}$ is the liquid saturation
- $S_{l, r}$ is the liquid residual saturation: for $S_{l} \leq S_{l, r}$ the relative permeability is zero
- $m$ is the van Genuchten exponent

See the [hysteresis page](hysteresis.md) for details concerning the wetting relative permeability.

To use this Material, it is necessary to include a [PorousFlowHysteresisOrder](PorousFlowHysteresisOrder.md) Material, which computes and records information regarding the hysteresis order and the saturation turning points.

!syntax parameters /Materials/PorousFlowHystereticRelativePermeabilityLiquid

!syntax inputs /Materials/PorousFlowHystereticRelativePermeabilityLiquid

!syntax children /Materials/PorousFlowHystereticRelativePermeabilityLiquid

# SaturationDensityFunction

This function computes saturation density of either the liquid phase or vapor phase from a given
temperature function $T(\mathbf{x}, t)$:
\begin{equation}
  \rho_\text{sat}(\mathbf{x},t) = \rho(p_\text{sat}(T(\mathbf{x}, t)),T(\mathbf{x}, t)) \,,
\end{equation}
where the $p_\text{sat}(T)$ function is provided via the `p_sat(T)` API of
objects deriving from `TwoPhaseFluidProperties` and the $\rho(p,T)$ function is provided via the
'rho_from_p_T(p,T)' API of the liquid or vapor 'SinglePhaseFluidProperties' object.

!syntax parameters /Functions/SaturationDensityFunction

!syntax inputs /Functions/SaturationDensityFunction

!syntax children /Functions/SaturationDensityFunction

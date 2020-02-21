# SaturationPressureFunction

This function computes saturation pressure $p_\text{sat}$ from a given
temperature function $T(\mathbf{x}, t)$:
\begin{equation}
  p_\text{sat}(\mathbf{x}, t) = p_\text{sat}(T(\mathbf{x}, t)) \,,
\end{equation}
where the $p_\text{sat}(T)$ function is provided via the `p_sat(T)` API of
objects deriving from `TwoPhaseFluidProperties`.

!syntax parameters /Functions/SaturationPressureFunction

!syntax inputs /Functions/SaturationPressureFunction

!syntax children /Functions/SaturationPressureFunction

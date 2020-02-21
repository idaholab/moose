# SaturationTemperatureFunction

This function computes saturation temperature $T_\text{sat}$ from a given
pressure function $p(\mathbf{x}, t)$:
\begin{equation}
  T_\text{sat}(\mathbf{x}, t) = T_\text{sat}(p(\mathbf{x}, t)) \,,
\end{equation}
where the $T_\text{sat}(p)$ function is provided via the `T_sat(p)` API of
objects deriving from `TwoPhaseFluidProperties`.

!syntax parameters /Functions/SaturationTemperatureFunction

!syntax inputs /Functions/SaturationTemperatureFunction

!syntax children /Functions/SaturationTemperatureFunction

# GapPerfectConductance

!syntax description /BCs/GapPerfectConductance

!alert note title=Often Created by an Action
This object can be set up automatically by using the [ThermalContact](syntax/ThermalContact/index.md) action.

## Description

This class enforces that temperatures match across the gap.  Specifically, the temperature on the secondary surface will match the temperature on the primary surface.  This is accomplished through a penalty constraint.  The residual is
\begin{equation}
  r = k (T_s - T_p)
\end{equation}
where $k$ is the penalty value, $T_s$ is the temperature on the secondary surface, and $T_p$ is the temperature on the primary surface.

## Example Input Syntax

!listing test/tests/gap_perfect_transfer/perfect_transfer_gap.i block=ThermalContact

!syntax parameters /BCs/GapPerfectConductance

!syntax inputs /BCs/GapPerfectConductance

!syntax children /BCs/GapPerfectConductance

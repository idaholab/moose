# SCMInletConductionHeatRatePostprocessor

!syntax description /Postprocessors/SCMInletConductionHeatRatePostprocessor

## Overview

!! this comment introduces vertical space

The user needs to specify a subchannel problem, either a [QuadSubChannel1PhaseProblem.md] or a
[TriSubChannel1PhaseProblem.md]. The postprocessor calculates the heat rate that axial conduction
carries out of the assembly through the inlet, against the flow, [inlet_conduction], where $i$ is
the subchannel index, $k$ the thermal conductivity, $c_p$ the specific heat, $S$ the flow area and
$\Delta z$ the height of the first axial cell. Face values are the average of the two nodes, as in
the enthalpy equation.

\begin{equation}
\label{inlet_conduction}
\sum_{i} \left(\frac{k}{c_p}\right)_{i,face} S_{i,face}
\frac{h_i(1) - h_i(0)}{\Delta z}
\end{equation}

The inlet enthalpy is prescribed, so when the first axial cell is heated the enthalpy gradient
across the inlet face drives a conduction flux out of the domain. A positive value leaves the
assembly. This term closes the assembly energy balance, as described in
[Energy balance](subchannel_theory.md#energy_balance): the pin and duct heat rates equal the
enthalpy flow rate rise of [SCMTHPowerPostprocessor.md] plus this conduction loss.

Only [TriSubChannel1PhaseProblem.md] assembles axial heat conduction, so this postprocessor returns
a value of zero for a quadrilateral assembly.

## Example Input File Syntax

!listing /test/tests/problems/SFR/EBR-II/XX09_SS_SHRT17.i block=Postprocessors language=moose

!syntax parameters /Postprocessors/SCMInletConductionHeatRatePostprocessor

!syntax inputs /Postprocessors/SCMInletConductionHeatRatePostprocessor

!syntax children /Postprocessors/SCMInletConductionHeatRatePostprocessor

# SCMInletConductionHeatRatePostprocessor

!syntax description /Postprocessors/SCMInletConductionHeatRatePostprocessor

## Overview

!! this comment introduces vertical space

The user needs to specify a subchannel problem, either a [QuadSubChannel1PhaseProblem.md] or a
[TriSubChannel1PhaseProblem.md]. The postprocessor calculates the surface-integrated heat flux that
axial conduction carries out of the assembly through the inlet, against the flow, [inlet_conduction], where $i$ is
the subchannel index, $k$ the thermal conductivity, $c_p$ the specific heat, $S$ the flow area and
$\Delta z$ the height of the first axial cell. Face values use the weight of the interpolation scheme at $Pe = 0$, as
in the enthalpy equation.

\begin{equation}
\label{inlet_conduction}
\sum_{i} \left(\frac{k}{c_p}\right)_{i,face} S_{i,face}
\frac{h_i(1) - h_i(0)}{\Delta z}
\end{equation}

The inlet enthalpy is prescribed with the inlet temperature. Where the first axial cell is heated,
which happens when `unheated_length_entry` is zero, the enthalpy gradient across the inlet face is as
steep as the gradient the heating produces, and conduction carries heat down that gradient out of the
domain, against the flow. The flux is physical, not a discretization error: it converges under axial
refinement rather than growing, because refining the mesh resolves the inlet gradient instead of
changing it. An unheated entry length removes it, since the first cell then sits at the inlet
temperature and the boundary gradient vanishes. A positive value leaves the assembly.

The outlet needs no matching term: the conduction stencil of the last cell couples it only to the
cell below, so the outlet face is adiabatic. This neglects conduction into an outlet plenum, which is
not part of the domain, and the enthalpy the flow carries out of the outlet is already in the
advective term.

The inlet conduction closes the assembly energy balance, as described in
[Energy balance](subchannel_theory.md#energy_balance): the pin and duct heat rates equal the
enthalpy flow rate rise of [SCMTHPowerPostprocessor.md] plus this conduction loss.

Only [TriSubChannel1PhaseProblem.md] assembles axial heat conduction, so this postprocessor returns
a value of zero for a quadrilateral assembly. It evaluates the stencil of the implicit enthalpy
solve, and reports an error for the explicit solve (`implicit = false`), which conducts
temperature rather than enthalpy with a different stencil.

## Example Input File Syntax

!listing /test/tests/problems/SFR/EBR-II/XX09_SS_SHRT17.i block=Postprocessors language=moose

!syntax parameters /Postprocessors/SCMInletConductionHeatRatePostprocessor

!syntax inputs /Postprocessors/SCMInletConductionHeatRatePostprocessor

!syntax children /Postprocessors/SCMInletConductionHeatRatePostprocessor

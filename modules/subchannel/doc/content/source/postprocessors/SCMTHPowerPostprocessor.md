# SCMTHPowerPostprocessor

!syntax description /Postprocessors/SCMTHPowerPostprocessor

## Overview

!! this comment introduces vertical space

The user needs to specify a subchannel problem. Either a [QuadSubChannel1PhaseProblem.md] or a [TriSubChannel1PhaseProblem.md]. The postprocessor will calculate the net power that goes into the coolant based on the thermal-hydraulic balance of inlet and outlet [enthalpy_balance]. Where $i$ is the subchannel index.

\begin{equation}
\label{enthalpy_balance}
\sum_{i} \dot{m}_i(out) * h_i(out) - \sum_{i} \dot{m}_i(in) * h_i(in)
\end{equation}

## Example Input File Syntax

listing /test/tests/problems/SFR/sodium-19pin/test19_implicit.i block=Postprocessors language=moose

!syntax parameters /Postprocessors/SCMTHPowerPostprocessor

!syntax inputs /Postprocessors/SCMTHPowerPostprocessor

!syntax children /Postprocessors/SCMTHPowerPostprocessor

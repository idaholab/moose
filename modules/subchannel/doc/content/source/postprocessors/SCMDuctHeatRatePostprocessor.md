# SCMDuctHeatRatePostprocessor

!syntax description /Postprocessors/SCMDuctHeatRatePostprocessor

## Overview

!! this comment introduces vertical space

The user needs to specify a subchannel problem. Either a [QuadSubChannel1PhaseProblem.md] or a [TriSubChannel1PhaseProblem.md]. The postprocessor will calculate the heat flow $[W]$ that goes into the coolant through the duct, based on the aux variable duct_heat_flux $[W/m^2]$. It will integrate this variable over the duct surface.

## Example Input File Syntax

listing /test/tests/problems/SFR/sodium-19pin/test19_implicit.i block=Postprocessors language=moose

!syntax parameters /Postprocessors/SCMDuctHeatRatePostprocessor

!syntax inputs /Postprocessors/SCMDuctHeatRatePostprocessor

!syntax children /Postprocessors/SCMDuctHeatRatePostprocessor

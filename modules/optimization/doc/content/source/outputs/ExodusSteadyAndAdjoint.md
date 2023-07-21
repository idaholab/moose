# ExodusSteadyAndAdjoint

!syntax description /Outputs/ExodusSteadyAndAdjoint

## Overview

This Exodus output object has the features of [Exodus](Exodus.md) and allows for [SteadyAndAdjoint](SteadyAndAdjoint.md) solves to output a per-iteration output. This allows users to analyze the
evolution of the optimization process solution as it converges on optimal values.

A test example is listed below:

!listing optimizationreporter/point_loads/forward_and_adjoint_iteration_output.i block=Outputs

!syntax parameters /Outputs/ExodusSteadyAndAdjoint

!syntax inputs /Outputs/ExodusSteadyAndAdjoint

!syntax children /Outputs/ExodusSteadyAndAdjoint

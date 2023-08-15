# ExodusOptimizationSteady

!syntax description /Outputs/ExodusOptimizationSteady

## Overview

This Exodus output object has the features of [Exodus](Exodus.md) and allows for [SteadyAndAdjoint](SteadyAndAdjoint.md) and [Steady](Steady.md) solves to output a per-iteration output. This allows users to analyze the
evolution of the optimization process solution as it converges to optimal values.

A test example is listed below:

!listing outputs/exodus_optimization_steady/forward_and_adjoint_iteration_output.i block=Outputs

!syntax parameters /Outputs/ExodusOptimizationSteady

!syntax inputs /Outputs/ExodusOptimizationSteady

!syntax children /Outputs/ExodusOptimizationSteady

# HeatConduction3EqnDGKernel

This DG kernel implements heat conduction for the [single-phase flow model](thermal_hydraulics/theory_manual/vace_model/index.md), using a central difference approximation:

!equation
\pr{k \pd{T}{x} A}_{i+1/2} = -k_{i+1/2} \frac{T_{i+1} - T_{i}}{x_{i+1} - x_i} A_{i+1/2} \eqc

where

!equation
k_{i+1/2} \equiv \frac{1}{2} \pr{k_{i} + k_{i+1}} \eqp

!syntax parameters /DGKernels/HeatConduction3EqnDGKernel

!syntax inputs /DGKernels/HeatConduction3EqnDGKernel

!syntax children /DGKernels/HeatConduction3EqnDGKernel

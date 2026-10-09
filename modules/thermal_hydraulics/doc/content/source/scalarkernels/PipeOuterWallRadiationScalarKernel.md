# PipeOuterWallRadiationScalarKernel

## Overview

This class adds a radiative heat transfer term, exchanged with an ambient/surrounding
environment, to the outer-surface radial node of a pipe wall. Unlike
[PipeOuterWallAmbientTemperatureScalarKernel.md], [PipeOuterWallCoupledConvectiveTemperatureScalarKernel.md], and [PipeOuterWallHeatFluxScalarKernel.md], it does not implement the
axial or radial conduction terms of the wall node's equation; it is a purely additive
term, meant to be added alongside another scalar kernel acting on the same outer-surface
wall scalar variable (such as [PipeOuterWallAmbientTemperatureScalarKernel.md] and [PipeOuterWallCoupledConvectiveTemperatureScalarKernel.md]), analogous
to how [ODETimeDerivative.md] supplies the time-derivative term rather than the full
equation; it adds the following term for [!eqref](modules/thermal_hydraulics/theory_manual/path_integrated_incompressible_model/index.md#discretized_outer_pipe_temperature)

!equation
\frac{\sigma \, f \, \varepsilon \, F \, P_{o,w} \left( T_\infty^4 - T_{o,w}^4 \right)}{A_{o,w} \rho_w c_{p,w}}

where

- $\sigma$ is the Stefan-Boltzmann constant,
- $f$ is an optional functor by which to scale the term,
- $\varepsilon$ is the surface emissivity,
- $F$ is the view factor for the portion of the outer wall surface exposed to the radiative environment at the ambient temperature,
- $P_{o,w}$ is the total perimeter of this wall node,
- $T_\infty$ is the ambient/surrounding temperature,
- $T_{o,w}$ is this node's own temperature,
- $A_{o,w}$ is the cross-sectional area of this wall layer, and
- $\rho_w$ and $c_{p,w}$ are the wall material's density and specific heat, evaluated at $T_{o,w}$.

This kernel takes a solid properties object based on the
[ThermalSolidProperties.md] base class, used only to evaluate $\rho_w$ and $c_{p,w}$ at this
node's own temperature for the normalization above (the same object and `area` value
supplied to this node's other kernel should be used here, for consistency).

Some consideration should be given to the
[!param](/ScalarKernels/PipeOuterWallRadiationScalarKernel/is_implicit) parameter. This
term allows the user to select whether the solve should be done with the current or the
previous state values of functor properties.

!syntax parameters /ScalarKernels/PipeOuterWallRadiationScalarKernel

!syntax inputs /ScalarKernels/PipeOuterWallRadiationScalarKernel

!syntax children /ScalarKernels/PipeOuterWallRadiationScalarKernel

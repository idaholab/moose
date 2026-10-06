# PipeOuterWallRadiationScalarKernel

## Overview

This class adds a radiative heat transfer term, exchanged with an ambient/surrounding
environment, to the outer-surface radial node of a pipe wall. Unlike
[PipeOuterWallAmbientTemperatureScalarKernel.md] and
[PipeOuterWallCoupledConvectiveTemperatureScalarKernel.md], it does not implement the
axial or radial conduction terms of the wall node's equation; it is a purely additive
term, meant to be added alongside another scalar kernel acting on the same outer-surface
wall scalar variable (such as [PipeOuterWallAmbientTemperatureScalarKernel.md]), analogous
to how [ODETimeDerivative.md] supplies the time-derivative term rather than the full
equation.

!equation
\frac{du}{dt} = \ldots + \frac{\sigma \, f \, \varepsilon \, F \, P \left( T_\infty^4 - u^4
\right)}{A \rho c_p}

where

- $\sigma$ is the Stefan-Boltzmann constant,
- $f$ is an optional functor by which to scale the term,
- $\varepsilon$ is the surface emissivity,
- $F$ is the view factor,
- $P$ is the perimeter of this wall node exposed to the radiative environment,
- $T_\infty$ is the ambient/surrounding temperature,
- $u$ is this node's own temperature,
- $A$ is the cross-sectional area of this wall layer, and
- $\rho$ and $c_p$ are the wall material's density and specific heat, evaluated at $u$.

This follows the same convention as [ADRadiativeHeatFluxBC.md], including its parameter
names and default values (`view_factor = 1`, `scale = 1`,
`stefan_boltzmann_constant = 5.670367e-8`), adapted here to a lumped scalar-kernel
formulation rather than a boundary-integrated finite-element residual: a perimeter is
supplied explicitly, and the result is normalized by $A \rho c_p$ so that it combines
correctly with the other kernels (radial conduction, axial conduction, convective/ambient
exchange, and the time derivative) acting on the same wall temperature variable.

This kernel takes a solid properties object based on the
[ThermalSolidProperties.md] base class, used only to evaluate $\rho$ and $c_p$ at this
node's own temperature for the normalization above (the same object and `area` value
supplied to this node's other kernel should be used here, for consistency).

Some consideration should be given to the
[!param](/ScalarKernels/PipeOuterWallRadiationScalarKernel/is_implicit) parameter. This
term allows the user to select whether the solve should be done with the current or the
previous state values of functor properties.

!syntax parameters /ScalarKernels/PipeOuterWallRadiationScalarKernel

!syntax inputs /ScalarKernels/PipeOuterWallRadiationScalarKernel

!syntax children /ScalarKernels/PipeOuterWallRadiationScalarKernel

# PipeInnerWallTemperatureScalarKernel

!syntax description /ScalarKernels/PipeInnerWallTemperatureScalarKernel

## Overview

This object solves for the temperature of the inner-surface radial node of a pipe wall at
a single axial location, as part of a two-radial-node lumped model of the wall (an inner
node and an outer node, split at a diameter between the pipe's inner and outer diameter).
It requires a coupled variable outer-surface wall temperature, coupled variables for the
upstream and downstream inner-surface wall temperatures, and a coupled variable fluid
temperature, all given as (coupled [ScalarVariables](syntax/Variables/index.md)).

!equation
A \rho c_p \frac{du}{dt} = \frac{2 \pi k \left( T_{out} - u \right)}{\ln(D_o / D_i)} +
\frac{k A}{L} \left( \frac{T_u - u}{\Delta x_u} + \frac{T_d - u}{\Delta x_d} \right) +
h P_w \left( T_f - u \right) \,

where

- $A$ is the cross-sectional area of the inner layer of the pipe wall, $A = \frac{\pi}{4}
  \left( D_m^2 - D_i^2 \right)$,
- $D_i$, $D_o$, and $D_m$ are the pipe inner diameter, outer diameter, and the diameter
  splitting the wall between the inner and outer radial nodes, respectively,
- $\rho$, $c_p$, and $k$ are the wall material's density, specific heat, and thermal
  conductivity, evaluated at the node's own temperature,
- $L$ is the axial length of this node's control volume,
- $T_{out}$ is the outer-surface wall node temperature,
- $T_u$ and $T_d$ are the upstream and downstream inner-surface wall node temperatures,
- $\Delta x_u$ and $\Delta x_d$ are the axial spacings to the upstream and downstream
  nodes,
- $h$ is the fluid-side heat transfer coefficient,
- $P_w$ is the fluid-wetted perimeter, and
- $T_f$ is the fluid temperature.

This kernel takes a solid properties object based on the
[ThermalSolidProperties](ThermalSolidProperties.md) base class. All geometry and
heat-transfer inputs are defined as functors, allowing the pipe's inner/outer diameter and
node spacing to vary along the axial direction.

Some consideration should be given to the
[!param](scalarkernels/PipeInnerWallTemperatureScalarKernel/is_implicit) parameter. This
term allows the user to select whether the solve should be done with the current or the
previous state values of functor properties.

As a reminder, the system of variables should be defined with the
[!param](/Variables/family) attribute set to `SCALAR` for each variable.

!syntax parameters /ScalarKernels/PipeInnerWallTemperatureScalarKernel

!syntax inputs /ScalarKernels/PipeInnerWallTemperatureScalarKernel

!syntax children /ScalarKernels/PipeInnerWallTemperatureScalarKernel

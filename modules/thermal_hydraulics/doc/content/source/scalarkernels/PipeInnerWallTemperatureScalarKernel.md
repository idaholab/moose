# PipeInnerWallTemperatureScalarKernel

!syntax description /ScalarKernels/PipeInnerWallTemperatureScalarKernel

## Overview

This object solves for the temperature of the inner-surface radial node of a pipe wall at
a single axial location, as part of a two-radial-node lumped model of the wall (an inner
node and an outer node, split at the diameter midway between the pipe's inner and outer
diameter). It couples to scalar variables for the outer-surface wall temperature, the
upstream and downstream inner-surface wall temperatures, the fluid temperature, and the
mass flow rate, all given as coupled [ScalarVariables](syntax/Variables/index.md).

!equation
A \rho c_p \frac{du}{dt} = \frac{2 \pi k \left( T_{out} - u \right)}{\ln(D_o / D_i)} +
\frac{G_u \left( T_u - u \right) + G_d \left( T_d - u \right)}{L} +
h \cdot 2 D_i \left( T_f + T_{in} - 2u \right)

where

- $A = \frac{\pi}{4} \left( D_m^2 - D_i^2 \right)$ is the cross-sectional area of the inner
  layer of the pipe wall, with $D_i$ and $D_o$ the pipe inner and outer diameters and
  $D_m = (D_i + D_o)/2$ the diameter splitting the wall between the inner and outer radial
  nodes,
- $\rho$, $c_p$, and $k$ are the wall material's density, specific heat, and thermal
  conductivity, evaluated at this node's own temperature $u$,
- $T_{out}$ is the outer-surface wall node temperature,
- $T_u$ and $T_d$ are the upstream and downstream inner-surface wall node temperatures,
- $L$ is the axial length of this node's control volume,
- $h$ is the fluid-side convective heat transfer coefficient, and
- $T_f$ is the fluid temperature adjacent to this node.

$G_u$ and $G_d$ are the axial face conductances to the upstream and downstream nodes. Each
is the harmonic mean of two half-length conduction resistances in series across the face,
accounting for the upstream/downstream node potentially having a different pipe diameter
than this node:

!equation
G_u = \left( \frac{\Delta x_u}{2 k A} + \frac{\Delta x_u}{2 k_u A_u} \right)^{-1}

and similarly for $G_d$ using the downstream spacing $\Delta x_d$, where $\Delta x_u$ and
$\Delta x_d$ are the axial spacings to the upstream and downstream nodes, and $k_u$, $A_u$
(respectively $k_d$, $A_d$) are the wall conductivity and layer area evaluated at the
upstream (downstream) node's own temperature and diameters.

$h$ is computed internally from a Dittus-Boelter correlation:

!equation
h = 0.023 \, Re^{0.8} Pr^{0.4} \frac{k_f}{D_i}, \qquad
Re = \frac{G D_i}{\mu_f}, \qquad G = \frac{4 |m|}{\pi D_i^2}, \qquad
Pr = \frac{\mu_f c_{p,f}}{k_f}

where $m$ is the mass flow rate and $\mu_f$, $c_{p,f}$, and $k_f$ are the fluid viscosity,
specific heat, and thermal conductivity, evaluated at the reference pressure and the mean
of $u$ and $T_{in}$. $T_{in}$ upwinds the upstream/downstream fluid temperature according
to the sign of $m$:

!equation
T_{in} = \frac{1}{2} \left( 1 - \frac{|m|}{m} \right) T_{f,d} +
         \frac{1}{2} \left( 1 + \frac{|m|}{m} \right) T_{f,u}

This kernel takes a solid properties object based on
[ThermalSolidProperties](ThermalSolidProperties.md) and a fluid properties object based
on [SinglePhaseFluidProperties](SinglePhaseFluidProperties.md). All geometry inputs (this
node's and its upstream/downstream neighbors' inner and outer diameters, this node's
control-volume length, and the axial spacings to each neighbor) are defined as functors,
allowing them to vary along the axial direction.

Some consideration should be given to the
[!param](scalarkernels/PipeInnerWallTemperatureScalarKernel/is_implicit) parameter. This
term allows the user to select whether the solve should be done with the current or the
previous state values of functor properties.

As a reminder, the system of variables should be defined with the
[!param](/Variables/family) attribute set to `SCALAR` for each variable.

!syntax parameters /ScalarKernels/PipeInnerWallTemperatureScalarKernel

!syntax inputs /ScalarKernels/PipeInnerWallTemperatureScalarKernel

!syntax children /ScalarKernels/PipeInnerWallTemperatureScalarKernel

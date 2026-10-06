# PipeInnerWallTemperatureScalarKernel

## Overview

This class implements the right-hand side of the flux balance for the inner-surface
radial node of a pipe wall at a single axial location, as part of a two-radial-node
lumped model of the wall (an inner node and an outer node, split somewhere between the
pipe's inner and outer surfaces). The model is geometry-general: it is driven entirely by
functor inputs for areas, perimeters, and a radial conduction-path thickness, rather than
a circular inner/outer diameter, so it is not restricted to hollow circular
cross-sections. It requires a coupled variable mass flow rate, a coupled variable
outer-surface wall temperature, coupled variables for the upstream and downstream
inner-surface wall temperatures, and coupled variables for the fluid temperature and its
upstream/downstream values, all given as coupled
[ScalarVariables](syntax/Variables/index.md). It operates on the inner-surface wall
temperature, $u$:

!equation
0 = \frac{1}{A \rho c_p} \left[ \frac{k P_r \left( T_{out} - u \right)}{t_r} +
\frac{G_u \left( T_u - u \right) + G_d \left( T_d - u \right)}{L} +
h \frac{P_w}{2} \left( T_f + T_{in} - 2u \right) \right]

where

- $A$ is the cross-sectional area of this (inner) layer of the pipe wall,
- $\rho$, $c_p$, and $k$ are the wall material's density, specific heat, and thermal
  conductivity, evaluated at this node's own temperature $u$,
- $T_{out}$ is the outer-surface wall node temperature,
- $P_r$ is the perimeter of the radial interface between this (inner) node and the outer
  node, and $t_r$ is the conduction-path distance between the two nodes,
- $T_u$ and $T_d$ are the upstream and downstream inner-surface wall node temperatures,
- $L$ is the axial length of this node's control volume,
- $h$ is the fluid-side convective heat transfer coefficient,
- $P_w$ is the wetted perimeter of the fluid flow channel adjacent to this node, and
- $T_f$ is the fluid temperature adjacent to this node.

$G_u$ and $G_d$ are the axial face conductances to the upstream and downstream nodes. Each
is the harmonic mean of two half-length conduction resistances in series across the face,
accounting for the upstream/downstream node potentially having a different cross-section
than this node:

!equation
G_u = \left( \frac{\Delta x_u}{2 k A} + \frac{\Delta x_u}{2 k_u A_u} \right)^{-1}

and similarly for $G_d$ using the downstream spacing $\Delta x_d$, where $\Delta x_u$ and
$\Delta x_d$ are the axial spacings to the upstream and downstream nodes, and $k_u$, $A_u$
(respectively $k_d$, $A_d$) are the wall conductivity and layer area evaluated at the
upstream (downstream) node's own temperature and cross-section.

$h$ is computed internally from a Dittus-Boelter correlation using the hydraulic
diameter of the flow channel, $D_h = 4 A_f / P_w$, with $A_f$ the flow channel's
cross-sectional area:

!equation
h = 0.023 \, Re^{0.8} Pr^{0.4} \frac{k_f}{D_h}, \qquad
Re = \frac{G D_h}{\mu_f}, \qquad G = \frac{|m|}{A_f}, \qquad
Pr = \frac{\mu_f c_{p,f}}{k_f}

where $m$ is the mass flow rate and $\mu_f$, $c_{p,f}$, and $k_f$ are the fluid viscosity,
specific heat, and thermal conductivity, evaluated at the reference pressure and the mean
of $u$ and $T_{in}$. $T_{in}$ upwinds the upstream/downstream fluid temperature according
to the sign of $m$:

!equation
T_{in} = \frac{1}{2} \left( 1 - \frac{|m|}{m} \right) T_{f,d} +
         \frac{1}{2} \left( 1 + \frac{|m|}{m} \right) T_{f,u}

Note, use of this kernel with transient problems also necessitates the use of a
[ODETimeDerivative.md] (or its AD equivalent) with `coefficient = 1` on the same scalar
variable, which contributes the time derivative term, $\frac{du}{dt}$, so the two kernels
together form the complete transient equation.

This kernel derives from [PipeWallTemperatureScalarKernelBase.md], which implements the
axial and radial conduction terms shared by every radial node of the wall; this class adds
the convective heat exchange with the primary flowing fluid.

This kernel takes a solid properties object based on the
[ThermalSolidProperties.md] base class and a fluid properties object based on the
[SinglePhaseFluidProperties.md] base class. All geometry inputs (the flow channel's area
and wetted perimeter, this node's and its upstream/downstream neighbors' wall
cross-sectional areas, the radial interface perimeter and thickness, this node's
control-volume length, and the axial spacings to each neighbor) are defined as functors,
which should allow versatility in accepting a variety of input arguments. Furthermore,
being functors, it is possible for them to be controlled via
[Controls](syntax/Controls/index.md) as supplied
[Postprocessors](syntax/Postprocessors/index.md), for example.

Some consideration should be given to the
[!param](/ScalarKernels/PipeInnerWallTemperatureScalarKernel/is_implicit) parameter. This
term allows the user to select whether the solve should be done with the current or the
previous state values of functor properties. This may allow the system to evolve more
slowly which may avoid some issues with respect to divergence of particularly unstable
systems.

As a reminder, the system of variables should be defined with the
[!param](/Variables/family) attribute set to `SCALAR` for each variable.

!syntax parameters /ScalarKernels/PipeInnerWallTemperatureScalarKernel

!syntax inputs /ScalarKernels/PipeInnerWallTemperatureScalarKernel

!syntax children /ScalarKernels/PipeInnerWallTemperatureScalarKernel

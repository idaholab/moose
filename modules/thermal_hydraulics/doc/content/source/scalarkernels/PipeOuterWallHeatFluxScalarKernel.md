# PipeOuterWallHeatFluxScalarKernel

## Overview

This class implements the right-hand side of the flux balance for the outer-surface
radial node of a pipe wall at a single axial location, as part of a two-radial-node
lumped model of the wall (an inner node and an outer node, split somewhere between the
pipe's inner and outer surfaces). It is a complement of
[PipeInnerWallTemperatureScalarKernel.md] for the outer node, used when the pipe's outer
surface receives a prescribed heat flux (for example, from an external heater or an
upstream heat-transfer calculation), as opposed to
[PipeOuterWallAmbientTemperatureScalarKernel.md], which couples the outer node to a fixed
ambient environment by convection, or
[PipeOuterWallCoupledConvectiveTemperatureScalarKernel.md], which couples it to a second
flowing fluid. The model is geometry-general: it is driven entirely by functor inputs for
areas, perimeters, and a radial conduction-path thickness, rather than a circular
inner/outer diameter, so it is not restricted to hollow circular cross-sections. It
requires a coupled variable inner-surface wall temperature and coupled variables for the
upstream and downstream outer-surface wall temperatures, all given as coupled
[ScalarVariables](syntax/Variables/index.md). It operates on the outer-surface wall
temperature, $u$:

!equation
0 = \frac{1}{A \rho c_p} \left[ \frac{k P_r \left( T_{in} - u \right)}{t_r} +
\frac{G_u \left( T_u - u \right) + G_d \left( T_d - u \right)}{L} + q P_q \right]

where

- $A$ is the cross-sectional area of this (outer) layer of the pipe wall,
- $\rho$, $c_p$, and $k$ are the wall material's density, specific heat, and thermal
  conductivity, evaluated at this node's own temperature $u$,
- $T_{in}$ is the inner-surface wall node temperature,
- $P_r$ is the perimeter of the radial interface between this (outer) node and the inner
  node, and $t_r$ is the conduction-path distance between the two nodes,
- $T_u$ and $T_d$ are the upstream and downstream outer-surface wall node temperatures,
- $L$ is the axial length of this node's control volume,
- $q$ is the prescribed heat flux, positive into the wall, and
- $P_q$ is the perimeter of this node over which the heat flux is applied.

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

$q$ is supplied directly by the user as a functor (a constant, function, variable,
material, or postprocessor), following the same sign convention as
[HSBoundaryHeatFlux.md]; this kernel does not compute the heat flux internally from a
flow or radiation correlation, and $q$ does not depend on this node's own temperature,
so this kernel makes no contribution to the Jacobian beyond the shared radial/axial
conduction terms.

Note, use of this kernel with transient problems also necessitates the use of a
[ODETimeDerivative.md] (or its AD equivalent) with `coefficient = 1` on the same scalar
variable, which contributes the time derivative term, $\frac{du}{dt}$, so the two kernels
together form the complete transient equation.

This kernel derives from [PipeWallTemperatureScalarKernelBase.md], which implements the
axial and radial conduction terms shared by every radial node of the wall; this class adds
the prescribed heat flux applied to the non-radial side of the node.

This kernel takes a solid properties object based on the
[ThermalSolidProperties.md] base class. All geometry inputs (this node's and its
upstream/downstream neighbors' wall cross-sectional areas, the radial interface
perimeter and thickness, the heated perimeter, this node's control-volume length, and
the axial spacings to each neighbor) are defined as functors, which should allow
versatility in accepting a variety of input arguments. Furthermore, being functors, it
is possible for them to be controlled via [Controls](syntax/Controls/index.md) as
supplied [Postprocessors](syntax/Postprocessors/index.md), for example.

Some consideration should be given to the
[!param](/ScalarKernels/PipeOuterWallHeatFluxScalarKernel/is_implicit) parameter. This
term allows the user to select whether the solve should be done with the current or the
previous state values of functor properties. This may allow the system to evolve more
slowly which may avoid some issues with respect to divergence of particularly unstable
systems.

As a reminder, the system of variables should be defined with the
[!param](/Variables/family) attribute set to `SCALAR` for each variable.

!syntax parameters /ScalarKernels/PipeOuterWallHeatFluxScalarKernel

!syntax inputs /ScalarKernels/PipeOuterWallHeatFluxScalarKernel

!syntax children /ScalarKernels/PipeOuterWallHeatFluxScalarKernel

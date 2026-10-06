# PipeOuterWallAmbientTemperatureScalarKernel

## Overview

This class implements the right-hand side of the flux balance for the outer-surface
radial node of a pipe wall at a single axial location, as part of a two-radial-node
lumped model of the wall (an inner node and an outer node, split somewhere between the
pipe's inner and outer surfaces). It is the complement of
[PipeInnerWallTemperatureScalarKernel.md], used when the pipe's outer surface loses or
gains heat by convection to an ambient environment (as opposed to the inner node, which
exchanges heat with the flowing process fluid). The model is geometry-general: it is
driven entirely by functor inputs for areas, perimeters, and a radial conduction-path
thickness, rather than a circular inner/outer diameter, so it is not restricted to
hollow circular cross-sections. It requires a coupled variable inner-surface wall
temperature and coupled variables for the upstream and downstream outer-surface wall
temperatures, all given as coupled [ScalarVariables](syntax/Variables/index.md). It
operates on the outer-surface wall temperature, $u$:

!equation
0 = \frac{1}{A \rho c_p} \left[ \frac{k P_r \left( T_{in} - u \right)}{t_r} +
\frac{G_u \left( T_u - u \right) + G_d \left( T_d - u \right)}{L} +
\mathcal{H} P_a \left( T_\infty - u \right) \right]

where

- $A$ is the cross-sectional area of this (outer) layer of the pipe wall,
- $\rho$, $c_p$, and $k$ are the wall material's density, specific heat, and thermal
  conductivity, evaluated at this node's own temperature $u$,
- $T_{in}$ is the inner-surface wall node temperature,
- $P_r$ is the perimeter of the radial interface between this (outer) node and the inner
  node, and $t_r$ is the conduction-path distance between the two nodes,
- $T_u$ and $T_d$ are the upstream and downstream outer-surface wall node temperatures,
- $L$ is the axial length of this node's control volume,
- $\mathcal{H}$ is the ambient heat transfer coefficient,
- $P_a$ is the perimeter of this node exposed to the ambient environment, and
- $T_\infty$ is the ambient temperature.

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

$\mathcal{H}$ and $T_\infty$ are supplied directly by the user as functors (constants,
functions, variables, materials, or postprocessors), following the same ambient
convection convention as [ADConvectionHeatTransferBC.md] and
[HSBoundaryAmbientConvection.md]; this kernel does not compute an ambient heat transfer
coefficient internally from a flow correlation.

Note, use of this kernel with transient problems also necessitates the use of a
[ODETimeDerivative.md] (or its AD equivalent) with `coefficient = 1` on the same scalar
variable, which contributes the time derivative term, $\frac{du}{dt}$, so the two kernels
together form the complete transient equation.

This kernel derives from [PipeWallTemperatureScalarKernelBase.md], which implements the
axial and radial conduction terms shared by every radial node of the wall; this class adds
the convective heat exchange with the ambient environment.

This kernel takes a solid properties object based on the
[ThermalSolidProperties.md] base class. All geometry inputs (this node's and its
upstream/downstream neighbors' wall cross-sectional areas, the radial interface
perimeter and thickness, the ambient-exposed perimeter, this node's control-volume
length, and the axial spacings to each neighbor) are defined as functors, which should
allow versatility in accepting a variety of input arguments. Furthermore, being
functors, it is possible for them to be controlled via
[Controls](syntax/Controls/index.md) as supplied
[Postprocessors](syntax/Postprocessors/index.md), for example.

Some consideration should be given to the
[!param](/ScalarKernels/PipeOuterWallAmbientTemperatureScalarKernel/is_implicit) parameter. This
term allows the user to select whether the solve should be done with the current or the
previous state values of functor properties. This may allow the system to evolve more
slowly which may avoid some issues with respect to divergence of particularly unstable
systems.

As a reminder, the system of variables should be defined with the
[!param](/Variables/family) attribute set to `SCALAR` for each variable.

!syntax parameters /ScalarKernels/PipeOuterWallAmbientTemperatureScalarKernel

!syntax inputs /ScalarKernels/PipeOuterWallAmbientTemperatureScalarKernel

!syntax children /ScalarKernels/PipeOuterWallAmbientTemperatureScalarKernel

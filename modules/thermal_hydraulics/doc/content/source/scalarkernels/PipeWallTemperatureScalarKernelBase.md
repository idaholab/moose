# PipeWallTemperatureScalarKernelBase

## Overview

This object acts as a base class for [PipeInnerWallTemperatureScalarKernel.md],
[PipeOuterWallAmbientTemperatureScalarKernel.md], and
[PipeOuterWallCoupledConvectiveTemperatureScalarKernel.md]. It implements the physics shared
by every radial node of a two-radial-node lumped pipe wall model: axial conduction to the
upstream/downstream nodes of the same radial layer, and radial conduction to the node on the
other side of the wall. It does not implement the heat exchange on this node's non-radial
side (with a flowing fluid or a fixed ambient environment), which each derived class
provides.

The model is geometry-general: it is driven entirely by functor inputs for areas,
perimeters, and a radial conduction-path thickness, rather than a circular inner/outer
diameter, so it is not restricted to hollow circular cross-sections.

This object takes a solid properties object based on the [ThermalSolidProperties.md] base
class. All geometry inputs (this node's and its upstream/downstream neighbors' wall
cross-sectional areas, the radial interface perimeter and thickness, this node's
control-volume length, and the axial spacings to each neighbor) are defined as functors,
which should allow versatility in accepting a variety of input arguments. Furthermore,
being functors, it is possible for them to be controlled via
[Controls](syntax/Controls/index.md) as supplied
[Postprocessors](syntax/Postprocessors/index.md), for example.

Some consideration should be given to the `is_implicit` parameter. This term allows the
user to select whether the solve should be done with the current or the previous state
values of functor properties. This may allow the system to evolve more slowly which may
avoid some issues with respect to divergence of particularly unstable systems.

Note, use of a derived kernel with transient problems also necessitates the use of a
[ODETimeDerivative.md] (or its AD equivalent) with `coefficient = 1` on the same scalar
variable, which contributes the time derivative term, $\frac{du}{dt}$, so the two kernels
together form the complete transient equation.

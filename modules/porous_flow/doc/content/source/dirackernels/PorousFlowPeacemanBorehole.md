# PorousFlowPeacemanBorehole

A `PorousFlowPeacemanBorehole` is a special case of the general line sink in which a polyline (represented by a sequence of points) acts as a sink or source in the model.  Please see [sinks](sinks.md) for an extended discussion and examples.

!alert warning
The function given by [!param](/DiracKernels/PorousFlowPeacemanBorehole/bottom_p_or_t) is evaluated at the well bottom.  If a file is read in using [!param](/DiracKernels/PorousFlowPeacemanBorehole/point_file) to define the coordinates and weights of the PorousFlowPeacemanBorehole, the well bottom is assumed to be the last entry in this file and [!param](/DiracKernels/PorousFlowPeacemanBorehole/bottom_p_or_t) will be evaluated at the z-coordinate of the last entry in [!param](/DiracKernels/PorousFlowPeacemanBorehole/point_file).  It is an error if the first entry in the [!param](/DiracKernels/PorousFlowPeacemanBorehole/point_file) has a smaller z-coordinate than the last entry.

!alert note
The wellbore pressure along the borehole can be built in two ways.  By default, a single constant
[!param](/DiracKernels/PorousFlowPeacemanBorehole/unit_weight) (fluid density $\times$ gravity) is
used, exactly as described in [sinks](sinks.md).  If instead
[!param](/DiracKernels/PorousFlowPeacemanBorehole/unit_weight_fp) is supplied (together with
[!param](/DiracKernels/PorousFlowPeacemanBorehole/unit_weight_temperature) and
[!param](/DiracKernels/PorousFlowPeacemanBorehole/unit_weight_gravity)), the fluid unit weight is
instead computed at each borehole point from a temperature-dependent fluid density, and
integrated along the well &mdash; see [sinks](sinks.md) for the formula, its justification, and
the deliberate approximation made in its Jacobian (the residual always uses the fluid density
implied by the current nonlinear iterate's temperature, but the Jacobian does not differentiate
the wellbore pressure with respect to temperature, since that dependence couples this borehole
point's residual to temperature degrees of freedom in other elements along the well, which a
DiracKernel cannot assemble into).  `unit_weight` and `unit_weight_fp` are mutually exclusive,
and `unit_weight_fp` is not compatible with `function_of = temperature`.  These four new
parameters are deliberately not named `fp`/`gravity`/`temperature_variable` (even though that
would mirror convention elsewhere in PorousFlow) because those are common `[GlobalParams]` names
used by unrelated Darcy kernels or fluid-properties materials; an input file that sets `gravity`
or `fp` at the `[GlobalParams]` level would otherwise silently activate, or fail to validate,
this mode on every `PorousFlowPeacemanBorehole` in the input.

To report the flow rate at each individual borehole point (rather than only the well-wide total
available via [!param](/DiracKernels/PorousFlowPeacemanBorehole/SumQuantityUO)), set
[!param](/DiracKernels/PorousFlowPeacemanBorehole/PointFluxUO) to a
[`PorousFlowPointFluxQuantity`](PorousFlowPointFluxQuantity.md) UserObject, and read it out with a
[`PorousFlowPlotPointFluxQuantity`](PorousFlowPlotPointFluxQuantity.md) VectorPostprocessor.

!syntax parameters /DiracKernels/PorousFlowPeacemanBorehole

!syntax inputs /DiracKernels/PorousFlowPeacemanBorehole

!syntax children /DiracKernels/PorousFlowPeacemanBorehole

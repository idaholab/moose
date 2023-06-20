# PorousFlowPeacemanBorehole

A `PorousFlowPeacemanBorehole` is a special case of the general line sink in which a polyline (represented by a sequence of points) acts as a sink or source in the model.  Please see [sinks](sinks.md) for an extended discussion and examples.

!alert warning
The function given by [!param](/DiracKernels/PorousFlowPeacemanBorehole/bottom_p_or_t) is evaluated at the well bottom.  If a file is read in using [!param](/DiracKernels/PorousFlowPeacemanBorehole/point_file) to define the coordinates and weights of the PorousFlowPeacemanBorehole, the well bottom is assumed to be the last entry in this file and [!param](/DiracKernels/PorousFlowPeacemanBorehole/bottom_p_or_t) will be evaluated at the z-coordinate of the last entry in [!param](/DiracKernels/PorousFlowPeacemanBorehole/point_file).  It is an error if the first entry in the [!param](/DiracKernels/PorousFlowPeacemanBorehole/point_file) has a smaller z-coordinate than the last entry.

!syntax parameters /DiracKernels/PorousFlowPeacemanBorehole

!syntax inputs /DiracKernels/PorousFlowPeacemanBorehole

!syntax children /DiracKernels/PorousFlowPeacemanBorehole

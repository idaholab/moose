# BSplineCurveGenerator

!syntax description /Mesh/BSplineCurveGenerator

B-Splines are notably desirable because they respect the incoming and outgoing direction of the curve,
avoiding a discontinuity in the slope.

The spline shape parameters should be adapted to obtain the desired curve. The sharpness is notably defined here
as a measure of the proximity of the spline to a curve consisting of three orthogonal segments, two continuing from
the start and end direction, and one joining the two at the location where that segment would be of minimal length.

!syntax parameters /Mesh/BSplineCurveGenerator

!syntax inputs /Mesh/BSplineCurveGenerator

!syntax children /Mesh/BSplineCurveGenerator

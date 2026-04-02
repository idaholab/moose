# BSplineCurveGenerator

!syntax description /Mesh/BSplineCurveGenerator

(Clamped) B-Splines are notably desirable because they can respect the incoming and outgoing direction of the curve,
avoiding a discontinuity in the slope when connected to another line mesh at either end.

Note that the end direction is considered from the point of view of the incoming end-point connecting line, not from the point of view of the curve.
If considering the point of the view of the generated curve, use the opposite value of the end direction.

The spline shape parameters should be adapted to obtain the desired shape for the curve. The sharpness is notably defined here
as a measure of the proximity of the spline to a curve consisting of three orthogonal segments, two continuing from
the start and end direction, and one joining the two at the location where that segment would be of minimal length.

!syntax parameters /Mesh/BSplineCurveGenerator

!syntax inputs /Mesh/BSplineCurveGenerator

!syntax children /Mesh/BSplineCurveGenerator

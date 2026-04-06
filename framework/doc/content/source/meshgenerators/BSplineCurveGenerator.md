# BSplineCurveGenerator

!syntax description /Mesh/BSplineCurveGenerator

(Clamped) B-Splines are notably desirable because they can respect the incoming and outgoing direction of the curve,
avoiding a discontinuity in the slope when connected to another line mesh at either end.

Note that the end direction is considered from the point of view of the incoming end-point connecting line, not from the point of view of the curve.
If considering the point of the view of the generated curve, use the opposite value of the end direction.

The spline shape parameters should be adapted to obtain the desired shape for the curve. The sharpness is notably defined here
as a measure of the proximity of the spline to a curve consisting of three orthogonal segments, two continuing from
the start and end direction, and one joining the two at the location where that segment would be of minimal length.

The control points $(cp)_i$ are generated at both ends of the spline with:

!equation
cp_i = P + \dfrac{i}{N_{cp} - 1} s * d_{\text{P to int.}} * \vec{d};

with:

- $N_{cp}$ the total number of control points
- $s$ the sharpness
- $P$ the start / end point
- $\vec{d}$ the starting / ending direction
- $d_{\text{P to int.}}$ the distance from the start/end point to the closest point, e.g. the point on the start/end line (formed by the start / end point and the starting/ending direction) that is closest to the end/start line


!syntax parameters /Mesh/BSplineCurveGenerator

!syntax inputs /Mesh/BSplineCurveGenerator

!syntax children /Mesh/BSplineCurveGenerator

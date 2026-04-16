# BSplineCurveGenerator

!syntax description /Mesh/BSplineCurveGenerator

(Clamped) B-Splines are notably desirable because they can respect the incoming and outgoing direction of the curve,
avoiding a discontinuity in the slope when connected to another line mesh at either end.

Note that the end direction is considered from the point of view of the incoming end-point connecting line, not from the point of view of the curve.
If considering the point of the view of the generated curve, use the opposite value of the end direction.

The spline shape parameters should be adapted to obtain the desired shape for the curve. The sharpness is notably defined here
as a measure of the proximity of the spline to a curve consisting of three line segments, each orthogonal to its neighbor(s), two continuing from
the start and end direction, and one joining the two at the location where that last segment would be of minimal length.

The control points $(cp)_i$ are generated at both ends of the spline with:

!equation
cp_i = P + \dfrac{i}{N_{cp} - 1} \cdot s \cdot d_{\text{P to int.}} \cdot \vec{d};

with:

- $N_{cp}$ the total number of control points
- $s$ the sharpness
- $P$ the start / end point
- $\vec{d}$ the starting / ending direction
- $d_{\text{P to int.}}$ the distance from the start/end point to the closest point, e.g. the point on the start/end line (formed by the start / end point and the starting/ending direction) that is closest to the end/start line

!alert note
If the start and end direction are parallel, the generator will fall back to producing a circle if the vector from the start to the end point
is orthogonal to the shared direction and the two direction vectors are also equal. All other cases with parallel directions will error.

## Setting start and end points / directions from mesh boundaries

As an alternative to specifying start/end points and directions manually, the user may specify another mesh generator
and a boundary (sideset) on the mesh created by that mesh generator for the start / end point and direction.

In that case, the starting point is computed from the centroid of the boundary (as a sideset).
The starting direction can still be specified with the [!param](/Mesh/BSplineCurveGenerator/start_direction),
or if it is not specified, using the side-area-weighted sum of the side-vertex-average-normal of the side elements on the boundary,
which is essentially one way to compute the average boundary normal.

The behavior is similar for the ending point/direction.

!syntax parameters /Mesh/BSplineCurveGenerator

!syntax inputs /Mesh/BSplineCurveGenerator

!syntax children /Mesh/BSplineCurveGenerator

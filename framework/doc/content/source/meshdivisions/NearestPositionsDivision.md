# NearestPositionsDivision

!syntax description /MeshDivisions/NearestPositionsDivision

The number of divisions is simply equal to the number of positions in the [Positions](syntax/Positions/index.md)
object specified in the [!param](/MeshDivisions/NearestPositionsDivision/positions) parameter.

!alert note
For points lying within the standard tolerance of an internal boundary of the bins,
(equi-distance between the point and two positions in the `Positions` object) this object
will output a warning. If you do not mind the indetermination on which bins they belong to but do mind
that a warning is output, please reach out to a MOOSE (or any MOOSE app) developer.

!syntax parameters /MeshDivisions/NearestPositionsDivision

!syntax inputs /MeshDivisions/NearestPositionsDivision

!syntax children /MeshDivisions/NearestPositionsDivision

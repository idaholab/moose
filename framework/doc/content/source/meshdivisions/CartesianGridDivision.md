# CartesianGridDivision

!syntax description /MeshDivisions/CartesianGridDivision

The number of mesh divisions/regions is the product of the number of bins in each direction.

Points that lay outside the Cartesian grid may be assigned to the grid outer bins
using the [!param](/MeshDivisions/CartesianGridDivision/assign_domain_outside_grid_to_border)
parameter.

!alert note
For points laying within the standard tolerance of an internal boundary of the Cartesian grid, this object
will output a warning. If you do not mind the indetermination on which bins they belong to but do mind
that a warning is output, please reach out to a MOOSE (or any MOOSE app) developer.

!syntax parameters /MeshDivisions/CartesianGridDivision

!syntax inputs /MeshDivisions/CartesianGridDivision

!syntax children /MeshDivisions/CartesianGridDivision

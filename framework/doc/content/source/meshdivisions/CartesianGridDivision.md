# CartesianGridDivision

!syntax description /MeshDivisions/CartesianGridDivision

The number of mesh divisions/regions is the product of the number of bins in each direction.

Points that lie outside the Cartesian grid may be assigned to the grid outer bins
using the [!param](/MeshDivisions/CartesianGridDivision/assign_domain_outside_grid_to_border)
parameter.

Using a [Positions](syntax/Positions/index.md) object as the [!param](/MeshDivisions/CartesianGridDivision/center_positions)
parameter, multiple Cartesian grids can be created around each position computed by that object. The division index
returned is for a point closest to position $i$ is then:

!equation
\text{division index} = (i - 1) N_{\text{single division}} + \text{division index in Cartesian grid centered around position i}

with $N_{\text{single division}}$ the number of divisions for a single Cartesian grid, based on the number of rings and axial
discretization specified.

!alert note
For points lying within the standard tolerance of an internal boundary of the Cartesian grid, this object
will output a warning. If you do not mind the indetermination on which bins they belong to but do mind
that a warning is output, please reach out to a MOOSE (or any MOOSE app) developer.

!syntax parameters /MeshDivisions/CartesianGridDivision

!syntax inputs /MeshDivisions/CartesianGridDivision

!syntax children /MeshDivisions/CartesianGridDivision

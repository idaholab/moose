# HexagonalGridDivision

!syntax description /MeshDivisions/HexagonalGridDivision

Points that lie outside the hexagonal grid may be assigned to the grid outer bins
using the [!param](/MeshDivisions/HexagonalGridDivision/assign_domain_outside_grid_to_border)
parameter. This is the current treatment for background/duct regions around hexagonal pins in
an assembly.

Using a [Positions](syntax/Positions/index.md) object as the [!param](/MeshDivisions/HexagonalGridDivision/center_positions)
parameter, multiple hexagonal grids can be created around each position computed by that object. The division index
of a point is then:

!equation
\text{division index} = (i - 1) N_{\text{single division}} + \text{division index in hexagonal grid centered around position i}

with $i$ the index in the `Positions` object of the position nearest from the point and $N_{\text{single division}}$ the number of divisions for a single hexagonal grid, based on the number of rings and axial discretization specified.

!alert note
For points lying within the standard tolerance of an internal boundary of the hexagonal grid, this object
will output a warning. If you do not mind the indetermination on which bins they belong to but do mind
that a warning is output, please reach out to a MOOSE (or any MOOSE app) developer.

!syntax parameters /MeshDivisions/HexagonalGridDivision

!syntax inputs /MeshDivisions/HexagonalGridDivision

!syntax children /MeshDivisions/HexagonalGridDivision

# SphericalGridDivision

!syntax description /MeshDivisions/SphericalGridDivision

The number of mesh divisions/regions is the product of the number of bins in each coordinate, which currently only
includes the radial direction.

Points that lie outside the spherical grid may be assigned to the closest grid outer bins (on the inner or outer shell)
using the [!param](/MeshDivisions/SphericalGridDivision/assign_domain_outside_grid_to_border)
parameter.

Using a [Positions](syntax/Positions/index.md) object as the [!param](/MeshDivisions/CylindricalGridDivision/center_positions)
parameter, multiple cylindrical grids can be created around each position computed by that object. The division index
of a point is then:

!equation
\text{division index} = (i - 1) N_{\text{single division}} + \text{division index in cylindrical grid centered around position i}

with $i$ the index in the `Positions` object of the position nearest from the point and $N_{\text{single division}}$ the number of divisions for a single spherical grid, based on the number of rings specified.

!alert note
We have not implemented binning in the azimuthal nor toroidal coordinates nor restrictions in those angular coordinates.
This is a desirable extension of this object.

!alert note
For points lying within the standard tolerance of an internal boundary of the spherical grid, this object
will output a warning. If you do not mind the indetermination on which bins they belong to but do mind
that a warning is output, please reach out to a MOOSE (or any MOOSE app) developer.

!syntax parameters /MeshDivisions/SphericalGridDivision

!syntax inputs /MeshDivisions/SphericalGridDivision

!syntax children /MeshDivisions/SphericalGridDivision

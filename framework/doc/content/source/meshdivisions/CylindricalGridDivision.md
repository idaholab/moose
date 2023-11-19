# CylindricalGridDivision

!syntax description /MeshDivisions/CylindricalGridDivision

The number of mesh divisions/regions is the product of the number of bins in each coordinate.

Points that lie outside the cylindrical grid may be assigned to the closest grid outer bins
using the [!param](/MeshDivisions/CylindricalGridDivision/assign_domain_outside_grid_to_border)
parameter.

!alert note
The [!param](/MeshDivisions/CylindricalGridDivision/center) should be a point on the axis of the cylinder
that also corresponds to the reference for the minimal ([!param](/MeshDivisions/CylindricalGridDivision/cylinder_axial_min)) and maximal ([!param](/MeshDivisions/CylindricalGridDivision/cylinder_axial_max)) axial extent of the cylinder.

!alert note
We have not implemented restrictions in the azimuthal direction so the entire ($0$, $2 \pi$) arc will be split.
This is a desirable extension of this object.

!alert note
For points lying within the standard tolerance of an internal boundary of the cylindrical grid, this object
will output a warning. If you do not mind the indetermination on which bins they belong to but do mind
that a warning is output, please reach out to a MOOSE (or any MOOSE app) developer.

!syntax parameters /MeshDivisions/CylindricalGridDivision

!syntax inputs /MeshDivisions/CylindricalGridDivision

!syntax children /MeshDivisions/CylindricalGridDivision

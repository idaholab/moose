# CylindricalAverage

!syntax description /VectorPostprocessors/CylindricalAverage

## Description

CylindricalAverage computes the average of a variable over cylindrical
shells. The cylinder is defined by the midpoint and a vector along the cylinder
axis. The cylindrical shells are defined by a maximum radius and the number
of desired shells.

!alert warning
The average is an average over quadrature points! The specific weight / volume associated with each quadrature point is not taken into account. The `SpatialAverageBase::execute` routine should be modified for that purpose.

!alert note title=Vector names / CSV output column names
`CylindricalAverage` declares a vector for the average cylindrical shell radii, named `radius`, and a vector for each variable requested, named with the variable name.

!syntax parameters /VectorPostprocessors/CylindricalAverage

!syntax inputs /VectorPostprocessors/CylindricalAverage

!syntax children /VectorPostprocessors/CylindricalAverage

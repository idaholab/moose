# SphericalAverage

!syntax description /VectorPostprocessors/SphericalAverage

The distance computed for the binning is based on the location of the quadrature points.

!alert warning
The average is an average over quadrature points! The specific weight / volume associated with each quadrature point is not taken into account. The `SpatialAverageBase::execute` routine should be modified for that purpose.

The CSV data output consists of the following columns, ordered by column name:

- variable average value

- average of inner and outer radius of each shell of the sphere

!alert note title=Vector names / CSV output column names
`SphericalAverage` declares a vector for the average shell radii, named `radius`, and a vector for each variable requested, named with the variable name.
Prepend with the name of the `SphericalAverage` object to obtain the reporter name of each vector.

## Example input syntax

In this example, the average of the variable `c` is computed for `10` layers (=bins) of a sphere of radius `5` around the origin `0 0 0` (default).

!listing test/tests/vectorpostprocessors/spherical_average/spherical_average.i block=VectorPostprocessors

!syntax parameters /VectorPostprocessors/SphericalAverage

!syntax inputs /VectorPostprocessors/SphericalAverage

!syntax children /VectorPostprocessors/SphericalAverage

# LayeredExtremumMaterialProperty

!syntax description /UserObjects/LayeredExtremumMaterialProperty

## How to define the layers

See the [LayeredAverage.md] documentation.

## How to retrieve the result

The result of a `LayeredExtremumMaterialProperty` computation can be saved in an auxiliary variable using a
[SpatialUserObjectAux.md]. It can be output to a CSV file using a [SpatialUserObjectVectorPostprocessor.md].

## Additional computation options

See the [LayeredAverage.md] documentation for more sampling option. The averaging operation is replaced by
an extremum (min or max) operation.


## Example input syntax

In this example, the minimum of material property `mat` is taken over the whole domain in direction `y` over
ten layers. The result of this averaging in stored in the variable `layered_extremum` using a
[SpatialUserObjectAux.md], and output to a CSV file using a [SpatialUserObjectVectorPostprocessor.md]

!listing test/tests/userobjects/layered_extremum/layered_extremum_matprop.i block=UserObjects AuxKernels VectorPostprocessors

!syntax parameters /UserObjects/LayeredExtremumMaterialProperty

!syntax inputs /UserObjects/LayeredExtremumMaterialProperty

!syntax children /UserObjects/LayeredExtremumMaterialProperty

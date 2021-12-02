# LayeredAverage

!syntax description /UserObjects/LayeredAverage

## How to define the layers

The layer definition is handled by the `LayeredBase` class. It is summarized here.
One, or multiple, layers are defined by their direction (normal or axis), their number,
and their thicknesses.

The direction of the layers currently may only be along the three Cartesian axes, and can be
set using the [!param](/UserObjects/LayeredAverage/direction) parameter.

The number of layers is set using the [!param](/UserObjects/LayeredAverage/num_layers) parameter,
unless the [!param](/UserObjects/LayeredAverage/bounds) parameter (see below) is set.

The thicknesses of the layers can be set in three different, and separate, ways:

- they can be set directly with coordinates along the specified direction using the
  [!param](/UserObjects/LayeredAverage/bounds) parameter. This will simultaneously set the number
  of layers, equal to the number of bounds minus one.

- the minimum and maximum coordinates along the specified directions can be both specified to specify
  the total extent of equal thickness layers, using the [!param](/UserObjects/LayeredAverage/direction_min)
  and [!param](/UserObjects/LayeredAverage/direction_max) parameters

- a vector of bounding domains may be specified using the [!param](/UserObjects/LayeredAverage/layer_bounding_block).
  The minimum and maximum coordinates along specified direction, which sets the thickness for each layer,
  are then obtained by looking at the maximum coordinate in the specified bounding block (to set the layers' minimum)
  and the minimum coordinate in the bounding blocks (set the layers's maximum).


The layers may be restricted to elements in certain subdomains using the
[!param](/UserObjects/LayeredAverage/block) parameter.

## How to retrieve the result

The result of a `LayeredAverage` computation can be saved in an auxiliary variable using a
[SpatialUserObjectAux.md]. It can be output to a CSV file using a [SpatialUserObjectVectorPostprocessor.md].

## Additional computation options

Sampling parameters may be specified to :

- obtain the layer average directly, by setting the [!param](/UserObjects/LayeredAverage/sample_type) to
  `direct` (default)

- interpolate between layer averages, by setting the [!param](/UserObjects/LayeredAverage/sample_type) to
  `interpolate`.

- average between layers, by setting the [!param](/UserObjects/LayeredAverage/sample_type) to
  `average`. The [!param](/UserObjects/LayeredAverage/average_radius) parameter can be specified
  to tune the distance over which to average results.


Additionally, cumulative averages over layers, in the positive direction, can be computed by setting
the [!param](/UserObjects/LayeredAverage/cumulative) to `true`.

## Example input syntax

In this example, the average of variable `u` is taken over the whole domain in direction `y` over
two layers. The result of this averaging in stored is the variable `layered_average` using a
[SpatialUserObjectAux.md], and output to a CSV file using a [SpatialUserObjectVectorPostprocessor.md].

!listing test/tests/userobjects/layered_average/layered_average.i block=UserObjects AuxKernels VectorPostprocessors

!syntax parameters /UserObjects/LayeredAverage

!syntax inputs /UserObjects/LayeredAverage

!syntax children /UserObjects/LayeredAverage

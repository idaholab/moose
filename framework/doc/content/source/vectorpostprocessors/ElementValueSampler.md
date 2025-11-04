# ElementValueSampler

!syntax description /VectorPostprocessors/ElementValueSampler

This `VectorPostprocessor` is similar to [NodalValueSampler](NodalValueSampler.md),
but is used for sampling elemental variables instead of nodal variables. The
coordinate used for each sampling point is the centroid (vertex-average approximation) of the associated
element.

!alert note title=Vector names / CSV output column names
`ElementValueSampler` declares a vector for each spatial coordinate, (`x`, `y`, `z`), of the centroid of the element
along with its ID as well as a vector named after each variable sampled, containing the variable values.

!alert note title=General sampling
The `ElementValueSampler` samples elemental variables at the element centroids. For more flexible sampling,
use the [PositionsFunctorValueSampler.md].

!syntax parameters /VectorPostprocessors/ElementValueSampler

!syntax inputs /VectorPostprocessors/ElementValueSampler

!syntax children /VectorPostprocessors/ElementValueSampler

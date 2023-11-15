# FunctorBinnedValuesDivision

!syntax description /MeshDivisions/FunctorBinnedValuesDivision

The number of divisions is simply equal to the number of bins.

Points that lie outside the (min_value, max_value) interval may be assigned to the outer bins
using the [!param](/MeshDivisions/FunctorBinnedValuesDivision/assign_out_of_bounds_to_extreme_bins)
parameter.

The `FunctorBinnedValuesDivision` will bin elements using the `ElemArg` functor argument. Where this is evaluated depends on
the functor, but tends to be at the element vertex average (approximation of the centroid).
The `FunctorBinnedValuesDivision` will bin points using the `ElemPointArg` functor argument. The evaluation is also
functor-implementation dependent. The element is located using the mesh point locator. If the point is located near several
elements, then the one with the lowest ID is used.

!alert note
For points laying within the standard tolerance of an internal boundary of the bins, this object
will output a warning. If you do not mind the indetermination on which bins they belong to but do mind
that a warning is output, please reach out to a MOOSE (or any MOOSE app) developer.

!syntax parameters /MeshDivisions/FunctorBinnedValuesDivision

!syntax inputs /MeshDivisions/FunctorBinnedValuesDivision

!syntax children /MeshDivisions/FunctorBinnedValuesDivision

# MultiAppGeneralFieldFunctorTransfer

!syntax description /Transfers/MultiAppGeneralFieldFunctorTransfer

This transfer can use any [Functor](syntax/Functors/index.md) to compute the values to transfer.
This provides great flexibility in how to compute source values, but also creates difficults, notably
because both the interpolation and extrapolation behavior of a functor depend on its particular implementation.

## Interpolation behavior

When a target point (see [MultiAppGeneralFieldTransfer.md] for how target points for transfers are chosen)
is located inside a source application domain by the point locator associated with the source mesh,
the source functor is evaluated.

If the point locator only found a single element, the functor is evaluated with an `ElemPointArg` (see [Functor](syntax/Functors/index.md)
documentation for definition) with that element and the target point in the source application frame of reference.
If multiple elements are found, the functor is evaluated with each element, and a arithmetic average of all
evaluations is returned.

!alert note
If multiple source child applications are overlapping for the same target point, then a source
value conflict should be detected and a warning is output.

## Extrapolation behavior options

The following options are available when the target point is outside the source application's mesh or outside
the source functor's subdomain restriction of the mesh:

- [!param](/Transfers/MultiAppGeneralFieldFunctorTransfer/extrapolation_behavior) `= flat` will return the
  [!param](/Transfers/MultiAppGeneralFieldFunctorTransfer/extrapolation_constant) specified by the user (defaults to -1).
- [!param](/Transfers/MultiAppGeneralFieldFunctorTransfer/extrapolation_behavior) `= nearest-node` will return the functor
  value at the nearest node on the boundary of the functor's domain of definition. If the functor is not defined at nodes,
  this will likely cause an error when pre-computing these values.
- [!param](/Transfers/MultiAppGeneralFieldFunctorTransfer/extrapolation_behavior) `= nearest-elem` will return the functor
  value at the nearest element on the boundary of the functor's domain of definition. If the functor is not defined at elements (rare),
  this will likely cause an error when pre-computing these values.
- [!param](/Transfers/MultiAppGeneralFieldFunctorTransfer/extrapolation_behavior) `= evaluate_oob` will evaluate the functor
  with a `null` element and the target point in the source application frame of reference. Some functors, such as spatial user objects
  or functions, can often be evaluate out of bounds (oob). Others, like variables, will error on such evaluation.


## Source and target spatial restrictions

All the spatial restrictions implemented in [MultiAppGeneralFieldTransfer.md] are available to this class.
The block restriction of the source functors are respected in that the functors are never evaluated
outside their block restriction, unless [!param](/Transfers/MultiAppGeneralFieldFunctorTransfer/extrapolation_behavior) is
set to `evaluate_oob`.

## Caveats and known numerical issues

The items below are known difficulties shared with the [MultiAppGeneralFieldNearestLocationTransfer.md].
The `MultiAppGeneralFieldFunctorTransfer.md` is not affected by these issues when interpolating values (where the
source and target domains overlap) because it does not use a nearest-location there, it simply evaluates the functors.
However, when extrapolating, it uses the same algorithm as [MultiAppGeneralFieldNearestLocationTransfer.md] and
thus is subject to the same caveats.

!alert warning
Nearest-location algorithms are vulnerable to finite precision round-offs if multiple sources are exactly at the
same distance from a given target. This can affect repeatability of results.
Use the [!param](/Transfers/MultiAppGeneralFieldFunctorTransfer/search_value_conflicts)
parameter to uncover these issues.

The [!param](/Transfers/MultiAppGeneralFieldFunctorTransfer/num_nearest_points) allows for a
simple geometric mixing of extrapolation values of several nearest nodes/elements to the target points. This mixing is performed
in every origin problem independently, values from different child applications
(or from different processes within each application) will not be mixed together.

!alert warning
If [!param](/Transfers/MultiAppGeneralFieldFunctorTransfer/num_nearest_points) is more than 1, the results
will differ in parallel if the target locations are near the parallel process boundaries
on the origin app mesh. Use the [!param](/Debug/SetupDebugAction/output_process_domains) parameter to examine
process boundaries on Exodus/Nemesis output.

## Example Input File Syntax

In this example, a `MultiAppGeneralFieldFunctorTransfer` is used to transfervalues from  a spatial user object `to_sub` from
block '1' in the main app to block '1' in the child app `sub`, filling the variable `from_main`.

!listing test/tests/transfers/general_field/functor/user_object/subdomain/main.i block=UserObjects Transfers/to_sub

!syntax parameters /Transfers/MultiAppGeneralFieldFunctorTransfer

!syntax inputs /Transfers/MultiAppGeneralFieldFunctorTransfer

!syntax children /Transfers/MultiAppGeneralFieldFunctorTransfer

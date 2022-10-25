# MultiAppGeneralFieldNearestNodeTransfer

!syntax description /Transfers/MultiAppGeneralFieldNearestNodeTransfer

The nearest neighbor may be the nearest node or the nearest quadrature point, depending on the context
such as the finite element/volume types of the source and target variables.
This object derived from the [MultiAppGeneralFieldTransfer.md] family of transfers and inherits
many of its features and characteristics.

Nearest node transfers may be preferred to [shape evaluation transfers](MultiAppGeneralFieldShapeEvaluationTransfer.md)
when extrapolation of data, e.g. when the target domain extends beyond the source domain, is required.
It may also be preferred if evaluating shape functions or projecting variables is too expensive or
unnecessary for the target application.

!alert warning
If [!param](/Transfers/MultiAppGeneralFieldNearestNodeTransfer/num_nearest_points) is more than 1, the results
will differ in parallel if the target locations are near the parallel process boundaries
on the origin app mesh. Use the [!param](/Debug/SetupDebugAction/output_process_domains) parameter to examine
process boundaries on Exodus/Nemesis output.

!alert note
This is a re-implementation of [MultiAppNearestNodeTransfer.md] using a more flexible algorithm.

!alert warning
Nearest-node algorithms are vulnerable to finite precision errors if multiple neighbors are exactly at the
same distance. This can affect repeatability of results.

## Example Input File Syntax

In this example, a `MultiAppGeneralFieldNearestNodeTransfer` is used to transfer a variable `to_sub` from
block '1' in the main app to block '1' in the child app, filling the variable `from_main`.

!listing test/tests/transfers/general_field/nearest_node/subdomain/main.i block=Transfers/to_sub

!syntax parameters /Transfers/MultiAppGeneralFieldNearestNodeTransfer

!syntax inputs /Transfers/MultiAppGeneralFieldNearestNodeTransfer

!syntax children /Transfers/MultiAppGeneralFieldNearestNodeTransfer

# MultiAppGeneralFieldNearestNodeTransfer

!syntax description /Transfers/MultiAppGeneralFieldNearestNodeTransfer

The nearest neighbor may be the nearest node or the nearest element centroid
(approximated by the vertex average), depending on the context
such as the finite element/volume types of the source and target variables.
This object derives from the [MultiAppGeneralFieldTransfer.md] family of transfers and inherits
many of its features and characteristics.

Nearest node transfers may be preferred to [shape evaluation transfers](MultiAppGeneralFieldShapeEvaluationTransfer.md)
when extrapolation of data is required, e.g. when the target domain extends beyond the source domain.
It may also be preferred if evaluating shape functions or projecting variables is too expensive or
unnecessary for the target application.

!alert note
This is a re-implementation of [MultiAppNearestNodeTransfer.md] using a more flexible algorithm.

!alert warning
Nearest-node algorithms are vulnerable to finite precision round-offs if multiple neighbors are exactly at the
same distance. This can affect repeatability of results. Use the [!param](/Transfers/MultiAppGeneralFieldNearestNodeTransfer/search_value_conflicts)
parameter to uncover these issues.

The [!param](/Transfers/MultiAppGeneralFieldNearestNodeTransfer/num_nearest_points) allows for a
simple geometric mixing of values of several nearest nodes to the target points. This mixing is performed
in every origin problem independently, values from different child applications
(or from different processes within each application) will not be mixed together.

!alert warning
If [!param](/Transfers/MultiAppGeneralFieldNearestNodeTransfer/num_nearest_points) is more than 1, the results
will differ in parallel if the target locations are near the parallel process boundaries
on the origin app mesh. Use the [!param](/Debug/SetupDebugAction/output_process_domains) parameter to examine
process boundaries on Exodus/Nemesis output.

## Example Input File Syntax

In this example, a `MultiAppGeneralFieldNearestNodeTransfer` is used to transfer a variable `to_sub` from
block '1' in the main app to block '1' in the child app `sub`, filling the variable `from_main`.

!listing test/tests/transfers/general_field/nearest_node/subdomain/main.i block=Transfers/to_sub

!syntax parameters /Transfers/MultiAppGeneralFieldNearestNodeTransfer

!syntax inputs /Transfers/MultiAppGeneralFieldNearestNodeTransfer

!syntax children /Transfers/MultiAppGeneralFieldNearestNodeTransfer

# MultiAppGeneralFieldNearestLocationTransfer

!syntax description /Transfers/MultiAppGeneralFieldNearestLocationTransfer

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
same distance. This can affect repeatability of results. Use the [!param](/Transfers/MultiAppGeneralFieldNearestLocationTransfer/search_value_conflicts)
parameter to uncover these issues.

The [!param](/Transfers/MultiAppGeneralFieldNearestLocationTransfer/num_nearest_points) allows for a
simple geometric mixing of values of several nearest nodes to the target points. This mixing is performed
in every origin problem independently, values from different child applications
(or from different processes within each application) will not be mixed together.

!alert warning
If [!param](/Transfers/MultiAppGeneralFieldNearestLocationTransfer/num_nearest_points) is more than 1, the results
will differ in parallel if the target locations are near the parallel process boundaries
on the origin app mesh. Use the [!param](/Debug/SetupDebugAction/output_process_domains) parameter to examine
process boundaries on Exodus/Nemesis output.

## Performance Considerations

Nearest-location transfers are usually fastest when the search can reject most source data before
building or querying the nearest-neighbor search. Prefer the most restrictive correct source and
target domains: use [!param](/Transfers/MultiAppGeneralFieldNearestLocationTransfer/from_blocks),
[!param](/Transfers/MultiAppGeneralFieldNearestLocationTransfer/to_blocks),
[!param](/Transfers/MultiAppGeneralFieldNearestLocationTransfer/from_boundaries), and
[!param](/Transfers/MultiAppGeneralFieldNearestLocationTransfer/to_boundaries) when only part of the
source or target mesh participates in the transfer.

For transfers involving many sub-applications, use the geometric search restrictions when they match
the problem setup. [!param](/Transfers/MultiAppGeneralFieldNearestLocationTransfer/use_nearest_position),
[!param](/Transfers/MultiAppGeneralFieldNearestLocationTransfer/from_mesh_division), and
[!param](/Transfers/MultiAppGeneralFieldNearestLocationTransfer/to_mesh_division) can reduce the
number of source points considered for each target point. If the nearest sub-application is known to
contain the nearest source location, setting
[!param](/Transfers/MultiAppGeneralFieldNearestLocationTransfer/assume_nearest_app_holds_nearest_location)
can avoid querying additional applications.

Keep [!param](/Transfers/MultiAppGeneralFieldNearestLocationTransfer/greedy_search) disabled for
production cases unless every source application must be checked. The
[!param](/Transfers/MultiAppGeneralFieldNearestLocationTransfer/search_value_conflicts) option is
useful for diagnosing equidistant source values, but it adds communication and comparison work and
should generally be disabled for large runs once the transfer is known to be well defined.

Bounding boxes are used to avoid unnecessary source queries. For extrapolating transfers, make sure
[!param](/Transfers/MultiAppGeneralFieldNearestLocationTransfer/bbox_factor) or
[!param](/Transfers/MultiAppGeneralFieldNearestLocationTransfer/fixed_bounding_box_size) is large
enough that valid source applications are not excluded, but avoid making the boxes much larger than
needed because that increases the number of candidate sources.

## Example Input File Syntax

In this example, a `MultiAppGeneralFieldNearestLocationTransfer` is used to transfer a variable `to_sub` from
block '1' in the main app to block '1' in the child app `sub`, filling the variable `from_main`.

!listing test/tests/transfers/general_field/nearest_node/subdomain/main.i block=Transfers/to_sub

!syntax parameters /Transfers/MultiAppGeneralFieldNearestLocationTransfer

!syntax inputs /Transfers/MultiAppGeneralFieldNearestLocationTransfer

!syntax children /Transfers/MultiAppGeneralFieldNearestLocationTransfer

# MultiAppGeneralFieldTransfer

Base class for all `GeneralField` transfers. It holds most setup and communication routines, leaving
to the derived classes the charge of computing the transferred values.

## General description

A `GeneralField` transfer proceeds as follows:

Each process first looks to find which source application it will be talking to.
This is based on geometric proximity, leveraging bounding boxes enclosing each application's
domain. The bounding boxes may need to be extended using the
[!param](/Transfers/MultiAppGeneralFieldNearestNodeTransfer/bbox_factor)
parameter to expand the domain considered for transfers with wider stencils.

Then each process shares the list of target points, locations it needs data for, with
each of the processes owning the source applications of interest. These other processes
then, using the behavior defined in the derived class, compute the local value of the source
variable. They also share geometric information about where this evaluation of the source
variable was located, which can be used for interpolation for example.

Finally, the data received is placed in the solution vector of the target variable(s).
This is performed using a local projection with the shape function of the target variable(s) in
order to support higher order variables.

## Features supported

All transfers derived from this base class should be able to support:

- block restriction in both the source and target application
- boundary restriction in both the source and target application
- arbitrary number of parallel processes for both the source and target application
- support for replicated and distributed meshes in both applications involved
- transfers between parent and child applications
- transfers between sibling applications (child to child)
- transfers between parent and multiple child applications in different locations
- transfers to and from a displaced mesh
- transfers of nodal and elemental variables
- transfers between variables of different finite element/volume family and type
- transfers between regular and array variables
- transfers from multiple variables to multiple variables
- interpolation and extrapolation transfers, as defined by the derived class
- detection of indetermination due to source points equidistant to a target point
- limitation of transfer source to the nearest position (see [Positions](syntax/Positions/index.md)) of target point


!alert note
Examine each derived object's respective documentation for feature support.

## Features not supported

The following features cannot be supported by general field transfers, as a limitation of
the `MultiAppGeneralFieldTransfer` base class.

- bi-directional transfer, a single transfer that send data to an app and from that same application
- transfers between two sibling `MultiApps` with different numbers of child applications
- reduction operations, sum/average/min/max, on data transferred from multiple child apps
- transfers between vector variables


These features are currently unsupported, but could be enabled if necessary with reasonable efforts:

- general coordinate transformations. Only the positions of the child apps are supported.
- caching optimizations for when both the target and origin mesh are constant


## Use of bounding boxes

Bounding boxes are used in general field transfers to perform very fast checks on
whether a point could belong to an origin mesh. If the point is not inside the bounding
box of an application's mesh, then it clearly cannot be inside that mesh. This allows to
disqualify a majority of origin meshes very fast in situations where the transfer
obtains data from multiple applications.

This process fails in several situations. When seeking to extrapolate from a source
application (or interpolate between non-overlapping source applications), the target points
can naturally be outside the bounding box of the source applications. In order to resolve this,
the `Transfer`'s [!param](/Transfers/MultiAppGeneralFieldNearestNodeTransfer/bbox_factor) and
[!param](/Transfers/MultiAppGeneralFieldNearestNodeTransfer/fixed_bounding_box_size) may be
used to inflate the bounding boxes.

!alert note
[!param](/MultiApps/TransientMultiApp/bounding_box_inflation) and
[!param](/MultiApps/TransientMultiApp/bounding_box_padding) parameters of the origin `MultiApp` are ignored.
Only the `MultiAppGeneralFieldTransfer` [!param](/Transfers/MultiAppGeneralFieldNearestNodeTransfer/bbox_factor)
and [!param](/Transfers/MultiAppGeneralFieldNearestNodeTransfer/fixed_bounding_box_size)
parameters are taken into account.

## Using the Positions system to restrict transfer sources

In addition to block and boundary restriction, the [Positions system](syntax/Positions/index.md) may
be used to match origin and target points. When specified with the
[!param](/Transfers/MultiAppGeneralFieldNearestNodeTransfer/use_nearest_position) parameter, each target
point of a transfer will only be matched with sources that are closest to the same `Position` as the
target point. The sources are simply the applications for:

- [MultiAppGeneralFieldUserObjectTransfer.md]
- [MultiAppGeneralFieldShapeEvaluationTransfer.md]

and nodes or centroids for:

- [MultiAppGeneralFieldNearestNodeTransfer.md]


!alert note
Unlike block and boundary restriction which are inclusive (more origin blocks/boundaries specified
means a larger source), specifiying more `Positions` further restricts the considered source for
a given target point.

!alert note
The "nearest position" criterion for the source of a transfer is obeyed strictly. If closer,
an invalid value (triggering the use of the [!param](/Transfers/MultiAppGeneralFieldUserObjectTransfer/extrapolation_constant))
should and will be preferred over a valid value.


## Overlap and floating point precision indetermination detection

The derived classes of `MultiAppGeneralFieldTransfer` may keep track of indetermination in origin values.
In the event of indetermination in origin values, a single value is still selected, usually the one from
the process with the lowest rank. These indetermination occur when for example:

- multiple points in the origin mesh(es) are equidistant to the target location, for nearest-node type transfers
- multiple child apps have an equidistant point to the target location, for nearest-node type transfers
- multiple child apps can compute a valid value for a target location because their meshes overlap

`MultiAppGeneralFieldTransfer` itself keeps track of indetermination by examining received values for each
target point. This occurs when multiple values are received for a single target point. Some transfers
examine the distance between origin and target points to select a value, but this can still lead to
indetermination if the distances are the same.

Mesh, block or boundary restriction can sometimes be used to alleviate these indeterminations
in the origin values. Other times, either remeshing one of the apps or using the
[!param](/MultiApps/TransientMultiApp/positions) parameter to create a very small offset can help
remove the indetermination.

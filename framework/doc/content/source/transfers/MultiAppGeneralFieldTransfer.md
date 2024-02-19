# MultiAppGeneralFieldTransfer

Base class for all `GeneralField` transfers. It holds most setup and communication routines, leaving
to the derived classes the charge of computing the transferred values.

## General description

A `GeneralField` transfer proceeds as follows:

Each process first looks to find which source application it will be talking to.
This is based on geometric proximity, leveraging bounding boxes enclosing each application's
domain. The bounding boxes may need to be extended using the
[!param](/Transfers/MultiAppGeneralFieldNearestLocationTransfer/bbox_factor)
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
- [mesh division](syntax/MeshDivisions/index.md) restriction in both the source and target application
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
- limitation of transfer source to the matching [mesh division](syntax/MeshDivisions/index.md) index between the source and target mesh divisions
- limitation of transfer source to subapps at the same index as the target mesh division
- limitation of transfer target to subapps at the same index as the source mesh division
- general coordinate transformations. Coordinate system changes (RZ to XYZ, for example) are not
  fully supported for the "floating point precision indetermination" detection.

!alert note
Examine each derived object's respective documentation for feature support.

!alert note
Floating point equidistance detection is turned on by default and will limit the scalability
of the transfer. Please set [!param](/Transfers/MultiAppGeneralFieldNearestLocationTransfer/search_value_conflicts)
to `false` for large cases.


## Features not supported

The following features cannot be supported by general field transfers, as a limitation of
the `MultiAppGeneralFieldTransfer` base class.

- bi-directional transfer, a single transfer that send data to an app and from that same application
- reduction operations, sum/average/min/max, on data transferred from multiple child apps
- transfers between vector variables


These features are currently unsupported, but could be enabled if necessary with reasonable efforts:

- caching optimizations for when both the target and origin mesh are constant

## Siblings transfer behavior

This transfer supports sending data from a MultiApp to a MultiApp with an arbitrary number
of source subapps in the source MultiApp and an arbitrary, possibly non-matching, number
of target subapps in the target MultiApp. It is the user's responsibility to ensure the
transfer is well defined, for example by avoiding overlaps between source multiapps which cause
multiple valid values for a target point.

## Use of bounding boxes

Bounding boxes are used in general field transfers to perform very fast checks on
whether a point could belong to an origin mesh. If the point is not inside the bounding
box of an application's mesh, then it clearly cannot be inside that mesh. This allows to
disqualify a majority of origin meshes very fast in situations where the transfer
obtains data from multiple applications.

This process fails in several situations. When seeking to extrapolate from a source
application (or interpolate between non-overlapping source applications), the target points
can naturally be outside the bounding box of the source applications. In order to resolve this,
the `Transfer`'s [!param](/Transfers/MultiAppGeneralFieldNearestLocationTransfer/bbox_factor) and
[!param](/Transfers/MultiAppGeneralFieldNearestLocationTransfer/fixed_bounding_box_size) may be
used to inflate the bounding boxes.
Note that the center of the bounding box is taken to be the center of the origin mesh's bounding box.

!alert note
[!param](/MultiApps/TransientMultiApp/bounding_box_inflation) and
[!param](/MultiApps/TransientMultiApp/bounding_box_padding) parameters of the origin `MultiApp` are ignored.
Only the `MultiAppGeneralFieldTransfer` [!param](/Transfers/MultiAppGeneralFieldNearestLocationTransfer/bbox_factor)
and [!param](/Transfers/MultiAppGeneralFieldNearestLocationTransfer/fixed_bounding_box_size)
parameters are taken into account.

## Using the Positions system to restrict transfer sources

In addition to block and boundary restriction, the [Positions system](syntax/Positions/index.md) may
be used to match origin and target points. When specified with the
[!param](/Transfers/MultiAppGeneralFieldNearestLocationTransfer/use_nearest_position) parameter, each target
point of a transfer will only be matched with sources that are closest to the same `Position` as the
target point. The sources are simply the applications for:

- [MultiAppGeneralFieldUserObjectTransfer.md]
- [MultiAppGeneralFieldShapeEvaluationTransfer.md]

and nodes or centroids for:

- [MultiAppGeneralFieldNearestLocationTransfer.md]


!alert note
Unlike block and boundary restriction which are inclusive (more origin blocks/boundaries specified
means a larger source), specifying more `Positions` further restricts the considered source for
a given target point.

!alert note
The "nearest position" criterion for the source of a transfer is obeyed strictly. If closer,
an invalid value (triggering the use of the [!param](/Transfers/MultiAppGeneralFieldUserObjectTransfer/extrapolation_constant))
should and will be preferred over a valid value.

!alert note
The origin and target locations for transferred data are both considered in the reference
domain (the one obtained by applying the coordinate transformation) when using the nearest positions
transfer options.

## Using mesh divisions in transfers

[Mesh divisions](syntax/MeshDivisions/index.md)
may be leveraged in several ways by specifying the [!param](/Transfers/MultiAppGeneralFieldNearestLocationTransfer/from_mesh_division_usage)
and the [!param](/Transfers/MultiAppGeneralFieldNearestLocationTransfer/to_mesh_division_usage)
parameters.

- Mesh divisions for spatial transfer restriction (usage = `spatial_restriction`)

Source and target mesh divisions, specified using the [!param](/Transfers/MultiAppGeneralFieldNearestLocationTransfer/from_mesh_division) and [!param](/Transfers/MultiAppGeneralFieldNearestLocationTransfer/to_mesh_division) parameters respectively, can be used
to limit the spatial domain that will provide data for the transfer (origin spatial restriction)
and the target spatial domain. The exclusion of a region is simply the region that is indexed with an invalid
index in the mesh division.

!alert note
The spatial restriction effect of the mesh divisions is active for all usages!
If some data lies outside the source mesh division, it will not be transferred.

- Matching regions using a source and target mesh divisions (usage = `matching_division`)

The source domain for the values to be transferred and the target domain can also be matched on
a one-to-one basis using the [!param](/Transfers/MultiAppGeneralFieldNearestLocationTransfer/from_mesh_division) and [!param](/Transfers/MultiAppGeneralFieldNearestLocationTransfer/to_mesh_division) parameters.
Each region of a given index into a division is matched to the region of the same index into the other
division.

It is advised to keep the number of mesh divisions the same in the two mesh divisions for simplicity.

- Matching source applications and target regions using a mesh division

Set [!param](/Transfers/MultiAppGeneralFieldNearestLocationTransfer/to_mesh_division_usage) to `matching_subapp_index`

The application providing values to transfer to a target region can be restricted to the
index of the points in the target region given by a mesh division specified by the [!param](/Transfers/MultiAppGeneralFieldNearestLocationTransfer/to_mesh_division) parameter.

It is advised to keep the number of target mesh divisions the same as the number of source subapps for simplicity.

- Matching target applications and source regions using a mesh division

Set [!param](/Transfers/MultiAppGeneralFieldNearestLocationTransfer/from_mesh_division_usage) to `matching_subapp_index`

The spatial region providing values to transfer to a target application can be restricted using a mesh division specified by the [!param](/Transfers/MultiAppGeneralFieldNearestLocationTransfer/from_mesh_division) parameter.
The target subapp will then only receive data that comes from the region at the same index, as the subapp,
in the source mesh division.

It is advised to keep the number of source mesh divisions the same as the number of target subapps for simplicity.

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

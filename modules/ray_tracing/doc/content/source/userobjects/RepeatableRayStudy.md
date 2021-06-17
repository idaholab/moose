# RepeatableRayStudy

!syntax description /UserObjects/RepeatableRayStudy

!alert note
The `RepeatableRayStudy` is meant to be the primary [RayTracingStudy.md] to be used for the majority of use cases and is not meant to be derived from! It is "repeatable" because it works with adaptivity and multiple executions (transients, on residual/Jacobian evaluations, etc). It does not require the user to have any knowledge of how to generate rays or determine on which processor and element element rays need to start depending on their starting point.

## Defining the Rays

The following parameters must first be set:

- [!param](/UserObjects/RepeatableRayStudy/names): A list of unique names to identify the rays being generated.
- [!param](/UserObjects/RepeatableRayStudy/start_points): A list of points that the rays should start from.

When using [RayKernels/index.md] and [RayBCs/index.md] with the `RepeatableRayStudy`, you must specify which rays the [RayKernels/index.md]/[RayBCs/index.md] are applied to via their own [!param](/UserObjects/RepeatableRayStudy/names) parameter. The names supplied to the [RayKernels/index.md] and [RayBCs/index.md] are the same as the names specified in your `RepeatableRayStudy`.

After setting these parameters, you must decide if you want to define the remainder of the trajectory by end points or by directions. +This decision is significant.+

!alert tip
Defining a Ray trajectory by end points (set with the [!param](/UserObjects/RepeatableRayStudy/end_points) parameter) is the recommended method for non-advanced cases. The rays will be traced until they reach the user-set end point. Example use cases of setting [Ray.md] trajectories via end points are line sources and line integrals, for which examples are found in [line_sources.md] and [line_integrals.md], respectively.

### Defining By End Points

To define the remainder of the trajectory by end points, provide the points at which you want the rays to end in the [!param](/UserObjects/RepeatableRayStudy/end_points) parameter. When the [Ray.md] end points are set, internally the tracer will set the max distance of each [Ray.md] individually such that they all end at the straight-line distance between the provided start point and the provided end point.

[RayKernels/index.md] and [RayBCs/index.md] can still end the rays earlier along their trajectory, but they are guaranteed to end once they hit either their end point or possibly sooner if the study's global maximum ray distance (the [!param](/UserObjects/RepeatableRayStudy/ray_distance) parameter) is less than the distance from the start to end point.

Rays that are killed due to reaching their max distance (which is the case when they reach their end point) are killed before the execution of [RayBCs/index.md]. For example, if a [Ray.md] reaches its end point and said end point is on a boundary with [RayBCs/index.md], the [RayBCs/index.md] will not be executed on the [Ray.md].

!alert note
Rays that have had their trajectory set via end points are not allowed to have their trajectories modified mid-trace via [RayKernels/index.md] or [RayBCs](RayBCs/index.md). For example, these rays cannot be reflected on boundaries via the [ReflectRayBC](ReflectRayBC.md). You must instead define rays by the [!param](/UserObjects/RepeatableRayStudy/directions) parameter if you want them to be able to have their trajectories changed mid-trace.

### Defining By Directions (Advanced Use)

The definition of [Ray.md] trajectory is considered advanced use because the user is responsible for ending the [Ray.md] by one of the following four methods:

- Killing it with a [RayBC](RayBCs/index.md) (example: [KillRayBC.md]).
- Killing it with a [RayKernel](RayKernels/index.md).
- Setting a maximum distance that it is allowed to travel via the [!param](/UserObjects/RepeatableRayStudy/max_distances) parameter.
- Setting a maximum distance that all rays are allowed to travel via the the [!param](/UserObjects/RepeatableRayStudy/ray_distance) parameter.

To define the trajectory by directions, provide the directions at which you want the rays to travel in the [!param](/UserObjects/RepeatableRayStudy/directions) parameter. These directions do not need to be normalized.

If the [Ray.md] intersects an external boundary and is not set to be killed and its trajectory is not changed, the tracer will error.

## Setting Ray Data

For more advanced use, one can also register [Ray.md] data/auxiliary data and initialize it as desired. It is important that this is not necessary when using [RayKernels/index.md] that contribute to residuals or integrate along lones, as the [Ray.md] data mangement in those cases is handled under the hood.

!syntax parameters /UserObjects/RepeatableRayStudy
  visible=Required Trajectory

!syntax inputs /UserObjects/RepeatableRayStudy

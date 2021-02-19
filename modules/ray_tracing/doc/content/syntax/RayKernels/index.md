# RayKernels

A RayKernel operates on a line segment of a [Ray.md] within the [ray_tracing/index.md], which is defined by the entry and exit point of the [Ray.md] within an element as it is traced through the mesh. The base object is the `RayKernelBase`.

Standard MOOSE convention denotes that a "kernel" is an object that contributes to the residual and the Jacobian. To remain consistent with this nomenclature, the `RayKernel` and `ADRayKernel` objects contribute to residuals and Jacobians from a [Ray.md].

A summary of the objects that should be derived based on desired operation on segments are as follows:

- [RayKernel.md]: Segments contribute to residuals and Jacobians (example: line sources, see [LineSourceRayKernel.md])
- [ADRayKernel.md]: Segments contribute to residuals and use automatic-differentiation to compute the Jacobian
- [GenericRayKernel.md]: Templated object that enables segment contribution to residuals and Jacobians with and without automatic differentiation (serves a similar purpose as [GenericKernel.md])
- [IntegralRayKernel.md]: Performs an integral on each segment and accumulates the integrated result into the [Ray.md] (example: integral of a variable along a line, see [VariableIntegralRayKernel.md])
- [AuxRayKernel.md]: Segments contribute to an AuxVariable in a user-defined manner (example: segment distances are accumulated into an AuxVariable, see [RayDistanceAux.md])
- [GeneralRayKernel.md]: General purpose object that can be adapted to operate on segments in a manner that is not covered by the classes listed above

The remainder of the discussion focuses on the use of the functionality offered within the base object, [RayKernelBase.md]. Refer to the objects above for a more specific discussion.

## Using a RayKernel

The method that is called on each segment of a [Ray.md] in a RayKernel is `onSegment()`. This method is to be overridden to specialize the on-segment operation. The `preTrace()` method is also available to be overridden and is called before a trace begins on a processor/thread.

The significant information pertaining to the trace that is available within `onSegment()` is as follows:

- `currentRay()` - The current [Ray.md] that is being traced on the segment.
- `_current_elem` - The current element that the [Ray.md] is being traced in.
- `_current_segment_start` - The start point of the current segment being operated on. This is not necessarily on the element periphery (a side of `_current_elem`) in the case that a [Ray.md] starts within an element.
- `_current_segment_end` - The end point of the current segment being operated on. This is not necessarily on the element periphery in the case that a [Ray.md] has ended within an element.
- `_current_segment_length` - The length of the current segment being operated on.
- `_current_intersected_side` - The side intersected on `_current_elem` at `_current_segment_end`, if any.
- `_current_incoming_side` - The side intersected on `_current_elem` at `_current_segment_end`, if any.
- `_current_intersected_extrema` - The extrema (element vertex or edge, see [ElemExtrema.md] for more information) intersected on `_current_intersected_side` at `_current_segment_end`, if any.
- `_current_subdomain_id` - The subdomain ID of the `_current_elem`.

See the [Ray.md] documentation for what members are available for use during tracing.

Many standard MOOSE interfaces are also available within RayKernels to do things like access coupled variables, access materials, access UserObjects, access Postprocessors, etc.

## Ending the Ray

`Ray::shouldContinue()` denotes whether or not a [Ray.md] will continue to be traced after execution of current objects on the [Ray.md]. In the case of a RayKernel, when `currentRay()->shouldContinue() == false`, the [Ray.md] will cease tracing after execution of RayKernels. Internally, the following will set the state of the [Ray.md] to not continue before RayKernels are executed but will still allow them to be executed on the segment that contains the final end point:

- If it has reached the user-set end point (when the [Ray.md] trajectory is set via `Ray::setStartingEndPoint()`). In this specific case, you can tell if the [Ray.md] has hit its end point by `currentRay()->atEnd()`.
- If it has reached the user-set maximum distance for the [Ray.md] (`currentRay()->distance() == currentRay()->maxDistance()`).
- If it has reached the global maximum distance set by the study parameter `ray_distance` (`currentRay()->distance() == _study.maxRayDistance()`)

To stop a [Ray.md] from being traced, call:

```
currentRay().setShouldContinue(false);
```

After a [Ray.md] has been set to not continue by any RayKernel, it cannot ever be set to continue again for the current trace.

## Changing the Ray Trajectory

A [Ray.md] that is currently being traced can have its trajectory changed mid-trace by a RayKernel. The following conditions are imposed on such a trajectory change:

- The [Ray.md] must be continuing (`currentRay()->shouldContinue() == true`). That is, it cannot have reached its max distance and it cannot have been set to not continue by another RayKernel. See [Ending the Ray](#ending-the-ray) for more information.
- The new start point (if changed) must remain within the element of the object that changed it.
- The [Ray.md] cannot have had its end point set by `Ray::setStartingEndPoint()`. That is, if the user set a specific end point for the [Ray.md] (which internally sets its maximum distance to the straight-line distance from the start to the end), its trajectory can never be changed.
- Only one trajectory change can be called by all RayKernels on each segment.
- The [Ray.md] must have moved before its trajectory is changed.

To change the trajectory of a [Ray.md] within a RayKernel, the method `changeRayStartDirection()` is to be called with parameters being the new start point of the [Ray.md] and the new direction of travel. For example:

```
const Point some_point(1, 2, 3); // must be within _current_elem!
const Point some_direction(1, 0, 0);
changeRayStartDirection(some_point, some_direction);
```

If the start point of the [Ray.md] is changed within the element, the [Ray.md] distance will be adjusted accordingly as if the [Ray.md] hit the changed point instead. That is, the original incremented distance will be removed (`_current_segment_length`) and the new incremented distance for this segment will be the distance between `currentRay()->currentPoint()` and `_current_segment_start`.

## Modifying/Registering Ray Data

If your RayKernel requires data or auxiliary data on a [Ray.md] that is unique to said object, you can register said requirement in the constructor using `_study.registerRayData()` and `_study.registerAuxData()`. For more information on [Ray.md] registration supplied by the [RayTracingStudy.md], see [RayTracingStudy.md#ray-data-registration].

An example of this exists in [IntegralRayKernel.md], which accumulates integrals into a data member on the [Ray.md]. Each [IntegralRayKernel.md] requires its own value to accumulate into, therefore the [IntegralRayKernel.md] registers a value for its own use:

!listing modules/ray_tracing/src/raykernels/IntegralRayKernel.C re=IntegralRayKernel::IntegralRayKernel.*?^}

!listing modules/ray_tracing/include/raykernels/IntegralRayKernel.h re=class IntegralRayKernel :.*?^};

By registering the data, the data is guaranteed to be available for all constructed [Rays](Ray.md) and can be accessed by `currentRay()->data()` and `currentRay()->auxData()`.

In the case of the [IntegralRayKernel.md], the [Ray.md] data is accessed as such:

!listing modules/ray_tracing/src/raykernels/IntegralRayKernel.C re=void\sIntegralRayKernel::onSegment.*?^}

## Creating Additional Rays

It is possible to generate another [Ray.md] to be traced from within a RayKernel. This method is thread safe.

First, acquire a new [Ray.md] using the `acquireRay()` method (for more information on acquiring [Rays](Ray.md), see [RayTracingStudy.md#ray-pool]), which takes as arguments the starting point and direction of travel for the new [Ray.md]. For example:

```
const Point some_point(1, 2, 3); // must be within _current_elem!
const Point some_direction(1, 0, 0);
std::shared_ptr<Ray> ray = acquireRay(some_point, some_direction);
```

The acquired [Ray.md] will be initialized with the following:

- Zeroed data and aux data, sized as registered by the `RayTracingStudy`.
- A starting point as set by the user.
- A starting element that is `_current_elem`.
- A unique ID.

After the [Ray.md] has been acquired, you may modify its data members as desired before setting it to be traced. Once the [Ray.md] is modified as desired, do the following to insert it into the buffer to be traced:

```
moveRayToBuffer(ray);
```

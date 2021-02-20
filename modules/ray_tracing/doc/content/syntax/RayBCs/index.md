# RayBCs

A RayBC operates on a [Ray.md] in the [ray_tracing/index.md] that has intersected a boundary (both external and internal boundaries are supported). The base object is the [RayBoundaryConditionBase.md].

Common use cases for a RayBC are as follows:

- Killing a [Ray.md] on a boundary (see [Killing the Ray](#killing-the-ray) and [KillRayBC.md]).
- Changing the direction of a [Ray.md] on a boundary, for example, reflecting the [Ray.md] (see [Changing the Ray Trajectory](#changing-the-ray-trajectory) and [ReflectRayBC.md]).
- Creating another [Ray.md] on a boundary mid-trace (see [Creating a New Ray](#creating-a-new-ray))

!alert tip
The use of RayBCs is considered +advanced+ use of the [ray_tracing/index.md]. For simple use cases like line sources (see [line_sources.md]) and line integrals (see [line_integrals.md]), RayBCs are not required.

Support is not currently available for contributing to residuals and Jacobians with RayBCs, nor is support for accessing coupled variables and materials on element sides in a RayBC. This support is planned in the future and will be implemented as a need arises.

## Using a RayBC

The method that is called on each intersected boundary along the trajectory of a [Ray.md] in a RayBC is `onBoundary()`. This method is to be overridden to specialize the on-boundary operation. For information on the `num_applying` parameter, see [Hitting Multiple Boundaries](#hitting-multiple-boundaries).

The significant information pertaining to the trace that is available within `onBoundary()` is as follows:

- `currentRay()` - The current [Ray.md] being operated on that has intersected the boundary.
- `_current_elem` - The element that the [Ray.md] has intersected on the boundary.
- `_current_intersected_side` - The side of `_current_elem` that the [Ray.md] has intersected on the boundary.
- `_current_intersection_point` - The point on `_current_elem` on `_current_intersected_side` that the [Ray.md] has intersected on the boundary.
- `_current_intersected_extrema` - The extrema (element vertex or edge, see [ElemExtrema.md] for more information) intersected on `_current_intersected_side` at `_current_intersection_point` on `_current_elem`, if any.
- `_current_bnd_id` - The ID of the boundary that the [Ray.md] has intersected.
- `_current_subdomain_id` - The subdomain ID of the `_current_elem`.

See the [Ray.md] documentation for what members are available for use during tracing.

## Killing the Ray

A RayBC can stop a [Ray.md] from being traced, as is done in [KillRayBC.md]:

!listing modules/ray_tracing/src/raybcs/KillRayBC.C re=void\sKillRayBC::onBoundary.*?^}

Similarly, you can check if another RayBC has set to kill a [Ray.md] after this segment with:

```
const bool ended = currentRay().shouldContinue();
```

After all RayBCs are executed at a point, if `!shouldContinue()`, the trace for the [Ray.md] will end.

## Changing the Ray Trajectory

A [Ray.md] that is currently being traced can have its trajectory changed on a boundary by a RayBC. The following conditions are imposed on a [Ray.md] with such a trajectory change:

- It must be continuing (`currentRay()->shouldContinue() == true`).
- It must have moved some before hitting the boundary (`currentRay()->distance() > 0`).
- The [Ray.md] cannot have had its end point set by `Ray::setStartingEndPoint()`. That is, if the user set a specific end point for the [Ray.md] (which internally sets its maximum distance to the straight-line distance from the start to the end), its trajectory can never be changed.
- The new direction must be entrant into `_current_elem` on `_current_intersected_side` (the dot product of the outward normal of the side and the direction of the [Ray.md] must be negative).

To change the trajectory of a [Ray.md] within a RayBC, the method `changeRayDirection()` is to be called with the new direction of the Ray. For example:

```
const Point some_direction(1, 0, 0); // must be entrant!
changeRayDirection(some_direction);
```

Note on the optional parameter `const bool skip_changed_check` in `changeRayDirection()`. By default, only one RayBC is allowed to change the trajectory of a [Ray.md] that is being traced that intersects a boundary. Said optional parameter sets this requirement. There exist cases when it is appropriate to change the trajectory of a [Ray.md] multiple times at a point. See [Hitting Multiple Boundaries](#hitting-multiple-boundaries) for more information.

## Hitting Multiple Boundaries

When hitting an element vertex in 2D and 3D or an element edge in 3D, it is possible to intersect multiple boundaries at the same point. This is supported and `onBoundary()` will be called once on each of the boundaries that are hit for every RayBC that is defined on said boundaries.

The `onBoundary()` method passes a single argument, `num_applying`. This argument denotes how many of the *same* RayBC object are being applied at a point. To explain the necessity of `num_applying`, consider the following problem:

- Two-dimensional domain with the boundaries `left`, `right`, `top`, and `bottom`.
- A single [ReflectRayBC.md] object is defined on all of the boundaries of the problem. That is, whenever a [Ray.md] hits any of the boundaries it will be reflected in a specular manner.
- One of the traced rays perfectly hits the top-right corner of the domain, at which both the `top` and `right` boundaries exist.

The correct specular reflection (inwards to the domain) can be achieved by applying a specular reflection on each of the boundaries, as seen below in Figure 1.

!media large_media/ray_tracing/raybc_multiple_reflect.png style=width:90%; caption=Figure 1: Example specular reflection on two boundaries.

 Recall that the act of changing a [Ray.md] direction, achieved by `changeRayDirection()`, takes an optional parameter `const bool skip_changed_check`. In this case, where we want to apply the same [ReflectRayBC.md] twice, we pass in `num_applying > 1` as the argument to `skip_changed_check` to *allow* the changing of a [Ray.md] trajectory multiple times if the same [ReflectRayBC.md] is applied more than once. This is done in [ReflectRayBC.md] as follows:

!listing modules/ray_tracing/src/raybcs/ReflectRayBC.C re=void\sReflectRayBC::onBoundary.*?^}

## Creating a New Ray

It is possible to generate another [Ray.md] to be traced from within a RayBC.

First, acquire a new [Ray.md] using the `acquireRay()` method (for more information on acquiring [Rays](Ray.md), see [RayTracingStudy.md#ray-pool]), which takes an argument that is the direction for the new [Ray.md]. For example:

```
const Point some_direction(1, 0, 0); // must be entrant on _current_intersected_side!
std::shared_ptr<Ray> ray = acquireRay(some_direction);
```

The acquired [Ray.md] will be initialized with the following:

- Zeroed data and aux data, sized as registered by the `RayTracingStudy`.
- A starting element that is `_current_elem`.
- An incoming side that is `_current_intersected_side`.
- A starting point that is `_current_intersected_point`.
- A direction as set by the user.
- A unique ID.

After the [Ray.md] has been acquired, you may modify its data members as desired before setting it to be traced. Once the [Ray.md] is modified as desired, do the following to insert it into the buffer to be traced:

```
moveRayToBuffer(ray);
```

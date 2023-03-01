# Ray

A `Ray` is the data structure that represents a single ray in the [ray_tracing/index.md] that is generated and traced by a [RayTracingStudy.md]. It can store data and auxiliary data that can modified along the trace via [RayKernels/index.md] and [RayBCs/index.md].

The discussion of how to use and interact with a `Ray` is summarized into the following sections:

- [Defining a Ray Trajectory](#defining-a-ray-trajectory): How to define a `Ray` trajectory for it to be traced using a [RayTracingStudy.md].
- [Modifying a Ray Trajectory](#modifying-a-ray-trajectory): How to modify a `Ray` trajectory while it is being traced via [RayKernels/index.md] and [RayBCs/index.md].
- [Using Ray Data](#using-ray-data): How to best access and use the data stored on the `Ray`.
- [Getting a Ray](#getting-a-ray): How to obtain a `Ray` to be used for tracing.

Useful member variables avaiable on the `Ray` are:

- `currentPoint()` - The current point of the `Ray`. Before being traced, this is the starting point. While being traced, this is the furthest point that the `Ray` has travelled (during RayKernel execution, this is the end of the segment). After being traced, this is the point where the `Ray` was killed.
- `currentElem()` - The current element that the `Ray` is in. Before being traced, this is the starting element of the `Ray`. During tracing during [RayKernel](RayKernels/index.md) execution, this is the element that the segment is in. During tracing during [RayBC](RayBCs/index.md) execution, this is the element that the [RayBC](RayBCs/index.md) is being applied to. At the end of tracing, this is the element that the `Ray` died in.
- `currentIncomingSide()` - The current incoming side on `currentElem()` that the `Ray` was incoming on. Before being traced, this is the side that the `Ray` will begin on (if any). During tracing, this is only valid when [RayKernels/index.md] are being executed. After tracing, this is not valid.
- `direction()` - The current direction of the `Ray` trajectory.
- `data()` - Access into the data stored on the `Ray`.
- `auxData()` - Access into the auxiliary data stored on the `Ray`.
- `distance()` - The total distance the `Ray` has traveled thus far.
- `maxDistance()` - The user-set maximum distance that this `Ray` can travel. When a user defines the `Ray` trajectory using `setStartingEndPoint()`, this is set internally to the straight-line distance from the start point to the user-set end point.
- `endSet()` - Whether or not the user defined the trajectory using the `setStartingEndPoint()` method. This identifies whether or not `maxDistance()` was set internally to ensure that the `Ray` ends at the user-defined end point.
- `endPoint()` - The user-set end point if it was set via `setStartingEndPoint()`.
- `shouldContinue()` - Whether or not the `Ray` should continue to be traced after [RayKernels/index.md] and [RayBCs/index.md] are executed.
- `setShouldContinue()` - Makes it possible to set a `Ray` to be killed after [RayKernels/index.md] and [RayBCs/index.md] are executed.
- `getInfo()` - Helper method for creating a `std::string` with useful information about the `Ray`.

## Defining a Ray Trajectory

A `Ray`'s trajectory defines where it is going to be traced by the [RayTracingStudy.md]. This description is *only* for defining a `Ray`'s trajectory before it is being traced.
To change a `Ray`'s trajectory mid-trace, see [Modifying a Ray Trajectory](#modifying-a-ray-trajectory).

First, you must define the starting point for the `Ray`. This is achieved via the `setStart()` method, which takes as arguments a starting point, an optional starting element, and an optional incoming side.

A starting element is required for a `Ray` to be traced, but there exist cases in which you will not initially set the starting element. For example, the [RepeatableRayStudyBase.md] will internally determine both the starting element and starting incoming side (if any) for a `Ray` via a claiming process.

Next, you must define where the `Ray` is to travel from its starting point. This is achieved by one of the following methods:

- `setStartingEndPoint()` - Takes as an argument the desired end point for the `Ray`. It will be traced until it hits said end point within the mesh (but it can be killed by [RayKernels/index.md] or [RayBCs/index.md] along the way). Internally, this is handled by setting `maxDistance()` to the straight-line distance from the start point to the end point. `Ray`s initialized using this method that have end points on the boundary will not have [RayBCs/index.md] executed on them on the boundary. They will be killed internally before the execution of [RayBCs/index.md].
- `setStartingDirection()` - Takes as an argument the desired direction for the `Ray` to travel. `Ray`s initialized by this method must be killed by either [RayKernels/index.md], by a [RayBCs/index.md], by the maximum distance `maxDistance()`, or by the [RayTracingStudy.md] maximum distance parameter `ray_max_distance`. If a `Ray` in this situation hits a boundary and is not killed, an error will be generated.

If you utilize the `setStartingDirection()` method, you may also utilize `setMaxDistance()` to set the maximum distance that the `Ray` is allowed to travel. If it reaches this distance, it will be killed after execution of [RayKernels/index.md]. If the `Ray` was initialized using the `setStartingEndPoint()` method, you cannot set the maximum distance for said `Ray` because it was set internally to the straight-line distance from the start to the user-set end.

!alert note
The generation of rays can easily become a very complicated task. When a `Ray` is added to the buffer to be traced, it must be on the processor that its starting element is on, or if on a processor boundary, on a neighboring processor to the starting element. It is recommended that you first see if the [RepeatableRayStudy.md] is sufficient for the generation of Rays for your use case.

## Modifying a Ray Trajectory

It is possible to modify the trajectory of a `Ray` while it is being traced via [RayKernels/index.md] and [RayBCs/index.md]:

To modify the trajectory of a `Ray` mid trace, see:

- [RayKernels/index.md#changing-the-ray-trajectory] for changing a `Ray`'s trajectory in a [RayKernel](RayKernels/index.md)
- [RayBCs/index.md#changing-the-ray-trajectory] for changing a `Ray`'s trajectory in a [RayBC](RayBCs/index.md)

## Using Ray Data

In its simplest form, `Ray` data is a vector of arbitrarily sized data and auxiliary data that lives on the `Ray` and remains until it is changed.

In order to ensure that said data is sized appropriately for the use case, a system exists to register the need for data in the [RayTracingStudy.md].
This guarantees that [RayTracingStudy.md] objects, all [RayBC](RayBCs/index.md) objects, and all [RayKernel](RayKernels/index.md) objects will have access to the data that they need on all `Ray`s that are traced. This registration devises an index into the `Ray` data and auxiliary data. For more information, see [RayTracingStudy.md#ray-data-registration].

With a specific data or auxiliary data index, the data are accessed using the `data()` and `auxData()` member variables on the `Ray`.

## Getting a Ray

The [RayTracingStudy.md] has a "pool" of `Ray` objects available for use. This pool allows for previously-allocated `Ray` objects that are no longer in use to be reset and re-used without deallocating and allocating memory again. This pool is the *only* way to construct new `Ray` objects.

For more information on the pool, see [RayTracingStudy.md#ray-pool].

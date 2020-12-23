# RayTracingMeshOutput

`RayTracingMeshOutput` is the base class for the output of rays traced with the [ray_tracing/index.md] as 1D elements in a mesh format. Exodus and Nemesis mesh formats are currently supported and are found in [RayTracingExodus.md] and [RayTracingNemesis.md], respectively. The information that follows pertains to both Exodus and Nemesis output---the only difference is the format in which the mesh is output. In order to output traced rays, you must enable the caching of the trace information in the [RayTracingStudy.md] (see [#trace-caching]).

Each segment of the [Ray.md] (the portion of the trace within an element) is represented as a 1D, edge element. This enables the overlaying of the traced rays on the actual mesh with ease using visualization software such as Paraview.

!! In the sections that follow, we have to cheat a bit and use derived classes (RayTracingExodus and RepeatableRayStudy) for the [!param] syntax because we can't use said syntax with base classes.

## Ray Information Output

Information on each segment can also be output. In the most general form, the information is output as a constant value on the Ray segment. The supported output for information on a [Ray.md] is as follows:

- All of the data on the [Ray](Ray.md), enabled by [!param](/Outputs/RayTracingExodus/output_data).
- All of the auxiliary data on the [Ray](Ray.md), enabled by [!param](/Outputs/RayTracingExodus/output_aux_data).
- The [Ray.md] ID, enabled by appending `ray_id` to the [!param](/Outputs/RayTracingExodus/output_properties) parameter.
- The number of intersections the [Ray.md] has encountered up to a segment, enabled by appending `intersections` to the [!param](/Outputs/RayTracingExodus/output_properties) parameter.
- The processor ID that the segment was executed on, enabled by appending `pid` to the [!param](/Outputs/RayTracingExodus/output_properties) parameter.
- The number of processor crossings that the [Ray.md] has encountered up to a segment, enabled by appending `processor_crossings` to the [!param](/Outputs/RayTracingExodus/output_properties) parameter.
- The number of trajectory changes that the [Ray.md] has encountered up to a segment, enabled by appending `trajectory_changes` to the [!param](/Outputs/RayTracingExodus/output_properties) parameter.

Note that the representation of data on a [Ray.md] segment as a constant value is an approximation. When this data is a represented as a constant value, the data is sampled at the furthest point of the trace after [RayKernels/index.md] and [RayBCs/index.md] are executed on it. Another form of approximation available is the output of this data in a nodal sense, where the data are sampled at both points on the segments and represented linearly. This is enabled by the
[!param](/Outputs/RayTracingExodus/output_data_nodal) parameter.

## Trace Caching

`RayTracingMeshOutput` uses cached information about the traces to produce this output. By default, this caching is not enabled in the [RayTracingStudy.md].

The default method in [RayTracingStudy.md] makes use of the [!param](/UserObjects/RepeatableRayStudy/always_cache_traces) parameter, which enables the caching of all traces. In addition, if you wish to output [Ray.md] data or auxiliary data, you must set true the [!param](/UserObjects/RepeatableRayStudy/data_on_cache_traces) and [!param](/UserObjects/RepeatableRayStudy/aux_data_on_cache_traces) parameters, respectively.

In simulations in which many rays are traced, storing and outputting each segment could be constrained by memory or could be computationally expensive. With this, there is also an option to represent the traces with as little information as possible. This option stores the cached information about the traces only when the [Ray.md] trajectory has changed or when the [Ray.md] has crossed a processor. This functionality is enabled by the [!param](/UserObjects/RepeatableRayStudy/segments_on_cache_traces) parameter in the [RayTracingStudy.md].

## Example

As an example, we are going to output the segment-wise accumulated integral for the simple diffusion variable integral example described in [ray_tracing/examples/line_integrals.md].

To recap, we begin with the "simple diffusion" problem:

!listing modules/ray_tracing/test/tests/raykernels/variable_integral_ray_kernel/simple_diffusion_line_integral.i start=Mesh end=Outputs

A [RepeatableRayStudy.md] is utilized that computes the integral of the variable $u$ from $(0, 0)$ to $(5, 5)$ and from $(5, 0)$ to $(5, 5)$:

!listing modules/ray_tracing/test/tests/raykernels/variable_integral_ray_kernel/simple_diffusion_line_integral.i start=UserObjects end=Postprocessors

Take note that we must enable the commented out parameters [!param](/UserObjects/RepeatableRayStudy/always_cache_traces) and [!param](/UserObjects/RepeatableRayStudy/data_on_cache_traces) in order to enable the caching of information needed for the output.

Lastly, we are going to add a [RayTracingExodus.md] output object that will output the traces in Exodus format. For Nemesis format, simply use [RayTracingNemesis.md].

!listing modules/ray_tracing/test/tests/raykernels/variable_integral_ray_kernel/simple_diffusion_line_integral.i start=Outputs end=UserObjects

In order to enable this output, we will set [!param](/UserObjects/RepeatableRayStudy/execute_on) to `TIMESTEP_END` as per the comment.

### Running the Example

First, we will run the example with constant data for the integrated value on each segment:

```
./ray_tracing-opt -i test/tests/raykernels/variable_integral_ray_kernel/simple_diffusion_line_integral.i UserObjects/study/always_cache_traces=true UserObjects/study/data_on_cache_traces=true Outputs/rays/output_data=true Outputs/rays/execute_on=TIMESTEP_END
```

Overlaying the standard output of the problem, `simple_diffusion_line_integral_out.e`, with the ray output, `simple_diffusion_line_integral_rays.e` and visualizing the `u_integral_value` variable on the ray output mesh, we obtain the result below in [constant].

!media large_media/ray_tracing/simple_diffusion_line_integral_rays_constant.png id=constant style=width:70%; caption=Simple diffusion line integral example result with Ray integral value overlay.

Second, we will instead plot the ray data in a nodal sense, in which the value on a segment is represented in a linear fashion with its start and end point values. This is done by enabling enabling [!param](/Outputs/RayTracingExodus/output_data_nodal), as:

```
./ray_tracing-opt -i test/tests/raykernels/variable_integral_ray_kernel/simple_diffusion_line_integral.iUserObjects/study/always_cache_traces=true UserObjects/study/data_on_cache_traces=true Outputs/rays/output_data_nodal=true Outputs/rays/execute_on=TIMESTEP_END
```

Again, overlaying the ray mesh result on the problem result we obtain the result below in [nodal].

!media large_media/ray_tracing/simple_diffusion_line_integral_rays_nodal.png id=nodal style=width:70%; caption=Simple diffusion line integral example result with Ray integral value overlay and nodal output enabled.

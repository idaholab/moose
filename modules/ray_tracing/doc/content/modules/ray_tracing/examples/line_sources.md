# Using Line Sources

The [ray_tracing/index.md] makes possible the contribution to residuals and Jacobians along segments of a line within the finite element mesh. The most simple use case of this capability is a source along a ray with given start and end points within the mesh.

## Example

We begin with the standard "simple diffusion" problem:

!listing modules/ray_tracing/test/tests/ray_kernels/line_source_ray_kernel/simple_diffusion_line_source.i start=Mesh end=UserObjects

### Defining the Study

A [RepeatableRayStudy.md] is defined that generates and executes the rays that compute the line source:

!listing modules/ray_tracing/test/tests/ray_kernels/line_source_ray_kernel/simple_diffusion_line_source.i start=UserObjects end=RayKernels

The `study` object defines a single [Ray.md] named `line_source_ray` that starts at (1, 1) and ends at (5, 2) to be executed on `PRE_KERNELS`.

!alert note
You must set `execute_on = PRE_KERNELS` for any studies that have [RayKernels/index.md] that contribute to residuals and Jacobians.

### Defining the RayKernel

[RayKernels/index.md] are objects that are executed on the segments of the rays. In this case, we wish to add a line source so we will define a [LineSourceRayKernel.md]:

!listing modules/ray_tracing/test/tests/ray_kernels/line_source_ray_kernel/simple_diffusion_line_source.i start=RayKernels end=Adaptivity

The `line_source` [LineSourceRayKernel.md] contributes to the variable `u` for [Ray.md] `line_source_ray` with a value of 5.

### Result

The visualized result follows in [result].

!media large_media/ray_tracing/simple_diffusion_line_source.png id=result style=width:70%; caption=Simple diffusion line source example result.

Just for the purposes of producing a more appealing picture, let's add some adaptivity to the mix to refine the region around the ray.

Take note of the `[Adaptivity]` block in the input file:

!listing modules/ray_tracing/test/tests/ray_kernels/line_source_ray_kernel/simple_diffusion_line_source.i start=Adaptivity

Setting the number of adaptivity steps to 5 via a command line argument, i.e.:

```
./ray_tracing-opt -i test/tests/ray_kernels/line_source_ray_kernel/simple_diffusion_line_source.i Adaptivity/steps=5
```

leads to the well-refined and visually satisfying result below in [result-adaptivity].

!media large_media/ray_tracing/simple_diffusion_line_source_adaptivity.png id=result-adaptivity style=width:70%; caption=Simple diffusion line source example result with adaptivity.

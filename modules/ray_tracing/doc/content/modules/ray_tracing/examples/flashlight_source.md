# Flashlight Point Sources

A "flashlight" point source is an anisotropic point source that emits in a cone about a direction $\hat{\Omega}$. The cone is defined by the vector along the cone axis and half of the opening angle in degrees.

The [ray_tracing/index.md] is utilized for this example to discretize the angular integration. An angular quadrature is sampled within the cone and a [Ray.md] generated for each direction. These [Rays](Ray.md) contribute to PDEs solved on the domain via line sources.

## Example

Consider a problem on a two-dimensional mesh in the physical domain $D \in [0, 5] \times [0, 5]$. It will contain a reaction term formed by the [Reaction.md] kernel and a diffusion term formed by the [Diffusion.md] kernel. The anisotropic point source located at $(1, 1.5)$ emits a source in a cone $2.5 \degree$ about  $\hat{\Omega} = (2 / \sqrt{5}, 1 / \sqrt{5})$. The right boundary will specularly reflect the point source.

The input begins with the basic definition (the mesh, the [Reaction.md] and [Diffusion.md] kernels, and an [Executioner/index.md]) as:

!listing modules/ray_tracing/test/tests/userobjects/cone_ray_study/cone_ray_study.i start=Mesh end=UserObjects

### Defining the Study

A [ConeRayStudy.md] is defined that generates and executes the rays within the cone:

!listing modules/ray_tracing/test/tests/userobjects/cone_ray_study/cone_ray_study.i start=UserObjects end=RayBCs

A summary of the set parameters is as follows:

- [!param](/UserObjects/ConeRayStudy/start_points) - The apex of the cone, defined as $(1, 1.5)$
- [!param](/UserObjects/ConeRayStudy/directions) - The direction of the center line of the cone, defined as $\hat{\Omega} = (2 / \sqrt{5}, 1 / \sqrt{5})$.
- [!param](/UserObjects/ConeRayStudy/half_cone_angles) - Half of the opening angle of the cone in degrees, defined as $2.5 \degree$
- [!param](/UserObjects/ConeRayStudy/ray_data_name) - A name for the data on the [Ray.md] to be registered that stores the weight from the angular quadrature
- [!param](/UserObjects/ConeRayStudy/execute_on) - Set to `PRE_KERNELS` so that the [Rays](Ray.md) are executed with the residual evaluation
- [!param](/UserObjects/ConeRayStudy/always_cache_traces) - Enabled to cache information for outputting the [Rays](Ray.md) in a mesh form (see [RayTracingMeshOutput.md])

### Defining the RayBCs

[RayBCs/index.md] are objects that operate on [Rays](Ray.md) that intersect a boundary. We require the following:

- A [RayBC](RayBCs/index.md) that reflects the rays on the right boundary in a specular manner
- A [RayBC](RayBCs/index.md) that ends the rays on the top boundary that are reflected off of the right boundary

In specific, we will add a [ReflectRayBC.md] on the right boundary and a [KillRayBC.md] on the top boundary. These are implemented as follows:

!listing modules/ray_tracing/test/tests/userobjects/cone_ray_study/cone_ray_study.i start=RayBCs end=RayKernels

At this point, it is important to note that it is not necessary to provide the [!param](/RayBCs/KillRayBC/rays) parameter to any added [RayKernels/index.md] or [RayBCs](RayBCs/index.md), unlike what was done in [line_sources.md] or [Computing Line Integrals](line_integrals.md). This is because the [ConeRayStudy.md] does not have [Ray.md] registration enabled (see [RayTracingStudy.md#ray-registration] for more information). [Ray.md] registration requires that a name be associated with each [Ray](Ray.md). It is disabled in the [ConeRayStudy.md] because the large number of [Rays](Ray.md) generated makes it unreasonable to name each one.

### Defining the RayKernel

[RayKernels/index.md] are objects that are executed on the segments of the [Rays](Ray.md). In this case, each [Ray.md] will act as a line source as it is traced. For this, we will add a [LineSourceRayKernel.md] as follows:

!listing modules/ray_tracing/test/tests/userobjects/cone_ray_study/cone_ray_study.i start=RayKernels/line_source end=Outputs

Take note of the supplied parameter [!param](/RayKernels/LineSourceRayKernel/ray_data_factor_names). Recall that within the defined [ConeRayStudy](ConeRayStudy.md), we supplied the parameter [!param](/UserObjects/ConeRayStudy/ray_data_name), which is the [Ray.md] data that will store the weights from the angular quadrature and the user defined scaling factors (if any). The [!param](/RayKernels/LineSourceRayKernel/ray_data_factor_names) parameter in [LineSourceRayKernel.md] will scale the source by the given [Ray.md] data, which effectively scales the line source term by the angular quadrature weights and the user defined scaling factors (if any).

### Result

The problem is ran with

```
./ray_tracing-opt -i test/tests/userobjects/cone_ray_study.i
```

and the result (in `cone_ray_study_out.e`) is pictured below in [result].

!media large_media/ray_tracing/cone_ray_study_coarse_u.png id=result caption=Result for the diffusion-reaction problem with a flashlight point source.

### Refined Result

Admittedly, the result in [result] is not particularly satisfying in a visual sense. We need more elements! Let's take advantage of the [syntax/Adaptivity/index.md]. With this, take note of the Adaptivity block:

!listing modules/ray_tracing/test/tests/userobjects/cone_ray_study/cone_ray_study.i start=Adaptivity

As in the input, the number of adaptivity steps is currently set to 0. We will set that to 6 via a command line argument and run again with:

```
mpiexec -n 4 ./ray_tracing-opt -i test/tests/userobjects/cone_ray_study.i Adaptivity/steps=6
```

!row!
!col small=12 medium=6 large=6
!media large_media/ray_tracing/cone_ray_study_u.png
       id=adaptivity
       caption=Refined result for the diffusion-reaction problem with a flashlight point source.

!col small=12 medium=6 large=6
!media large_media/ray_tracing/cone_ray_study_u_mesh.png
      id=adaptivity-meshed
      caption=The result pictured in [adaptivity] with a mesh overlay.
!row-end!

### Result with Ray Overlay

For a little more pretty-picture goodness, we will also output the generated [Rays](Ray.md) using [RayTracingMeshOutput.md].

Recall the [!param](/UserObjects/ConeRayStudy/always_cache_traces) parameter that was set to true for the [ConeRayStudy.md]. This was enabled so that the trace information could be cached for use in output. Take note of the Output block with the [RayTracingExodus.md] object:

!listing modules/ray_tracing/test/tests/userobjects/cone_ray_study/cone_ray_study.i start=Outputs end=Adaptivity

The output file `cone_ray_study_rays.e` is also generated with the simulation. We will overlay this mesh on top of the original output with adaptivity to obtain what follows in [adaptivity-rays].

!media large_media/ray_tracing/cone_ray_study_rays.png id=adaptivity-rays caption=Result for the diffusion-reaction problem with a flashlight point source, with adaptivity, and with a [RayTracingExodus.md] overlay.

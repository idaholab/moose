# AddRayKernelAction

!syntax description /RayKernels/AddRayKernelAction

For more information, see [RayBCs/index.md].

A [RayKernels/index.md](RayKernel) requires a [RayTracingStudy.md] to be associated with it. This is provided with the [!param](/RayKernels/NullRayKernel/study) parameter. If the [!param](/RayKernels/NullRayKernel/study) parameter not provided, the associated [RayTracingStudy.md] will be the one study object that exists. If multiple studies exist, the [!param](/RayKernels/NullRayKernel/study) parameter must be provided.

!syntax parameters /RayKernels/AddRayKernelAction

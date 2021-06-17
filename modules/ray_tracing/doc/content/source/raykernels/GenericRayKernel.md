# GenericRayKernel

Acts as a switch between the AD/non-AD base class for [RayKernels/index.md] that calculate residual contributions along the trace of a [Ray.md].

This effectively allows a class to either inherit from [RayKernel.md] for the non-AD case or [ADRayKernel.md] for the AD case via the boolean `<is_ad>`. For en example, see [LineSourceRayKernel.md].

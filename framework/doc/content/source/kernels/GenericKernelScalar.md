# GenericKernelScalar

GenericKernelScalar acts as a switch between the AD/non-AD base class for objects that
calculate residual vector contributions of volumetric (kernel) integrals on 
nonlinear scalar variables coupled with field variables.

This effectively allows a class to either inherit from [KernelScalarBase](KernelScalarBase.md) for
the non-AD or ADKernelScalarBase for the AD case via the boolean `<is_ad>`.

See [GenericKernel](GenericKernel.md) for a similar description for field variables.

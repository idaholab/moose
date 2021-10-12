# GenericKernel

GenericKernel acts as a switch between the AD/non-AD base class for objects that
calculate residual vector contributions of nonlinear scalar field variables.

This effectively allows a class to either inherit from [Kernel](Kernel.md) for
the non-AD or ADKernel for the AD case via the boolean `<is_ad>`.

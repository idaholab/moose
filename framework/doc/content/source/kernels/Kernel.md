# Kernel

Kernel is the base class for object that calculate residual vector contributions
of nonlinear scalar field variables.

Derived from it are two sets of optimized base classes that - through loop
reordering - reduce the number of necessary residual evaluations and enable the
compiler to perform code vectorization. These base classes have restrictions on
the mathematical form of the residuals they can be applied to.

- [KernelValue](/KernelValue.md)
- [ADKernelValue](/ADKernelValue.md)
- [KernelGrad](/KernelGrad.md)
- [ADKernelGrad](/ADKernelGrad.md)

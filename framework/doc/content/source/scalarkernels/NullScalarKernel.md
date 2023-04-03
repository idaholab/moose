# NullScalarKernel

!syntax description /ScalarKernels/NullScalarKernel

This kernel can be used as a placeholder if a nonlinear scalar variable was added to the simulation but
the kernels for that variable have not yet been created / added to the input file. This kernel
will ensure that the kernel coverage checking of the domain does not flag the variable as problematic.
It will also ensure that the system matrix remains invertible as it contributes a small number times
the identity matrix to the Jacobian for the variable it applies to.

## Alternatives

- Ideally, the non-linear scalar variable should be moved to be an [auxiliary variable](syntax/AuxVariables/index.md). This will reduce the size of the nonlinear system, making it easier to solve.

- The kernel block coverage checking of the domain can also be turned off by setting `kernel_coverage_check=false` in the `[Problem]` block.

- A time derivative kernel can be used for a non-linear scalar variable $v$ with the missing physics kernels, as this describes a valid equation $\dfrac{dv}{dt}=0$.


!alert note
This scalar kernel should mainly be used for creating small tests and while debugging simulations.

!syntax parameters /ScalarKernels/NullScalarKernel

!syntax inputs /ScalarKernels/NullScalarKernel

!syntax children /ScalarKernels/NullScalarKernel

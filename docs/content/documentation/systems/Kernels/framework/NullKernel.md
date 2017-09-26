<!-- MOOSE Documentation Stub: Remove this when content is added. -->

# NullKernel

## Description

True to its name, the `NullKernel` sets a zero residual. A `NullKernel` is used
when a variable would otherwise have no diagonal entries in its Jacobian
matrix. Without these entries, PetSc will fail with an error message like:

```
[0]PETSC ERROR: --------------------- Error Message -----------------------
[0]PETSC ERROR: Object is in wrong state
[0]PETSC ERROR: Matrix is missing diagonal entry 242
[0]PETSC ERROR: See http://www.mcs.anl.gov/petsc/documentation/faq.html for
trouble shooting.
```

Moreover, use of a `NullKernel` allows the user to keep the
`kernel_coverage_check` in place.

## Example Syntax

A good example of where `NullKernel` is necessary can be found in a phase field
module test for the Lagrange multiplier $\lambda$, which without the Jacobian
fill provided by the `NullKernel` would have a zero diagonal. The test is
reproduced below:

!listing modules/phase_field/test/tests/misc/equal_gradient_lagrange.i label=false

!syntax parameters /Kernels/NullKernel

!syntax inputs /Kernels/NullKernel

!syntax children /Kernels/NullKernel

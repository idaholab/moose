# FVMatAdvection

!syntax description /FVKernels/FVMatAdvection

The `FVMatAdvection` kernel is similar to the [FVAdvection.md] kernel except that:

- the velocity is a material property instead of a constant vector, so it may vary throughout the domain

- an advected quantity


In order to solve for the velocity, one needs to add a `VariableMaterial`, such as the
[`INSFVMaterial`](modules/navier_stokes/doc/content/source/materials/INSFVMaterial.md optional=True)
to store a copy of the velocity variable as a material property.

As we are expanding the functor material & variable capability, having the velocity as a material
property will no longer be required.

## Example input syntax

In this example, the kernels are set up for a steady state advection problem of both
momentum and mass. For the former the `advected_quantity` is set to the momentum. This
case uses a constant unit density. Please refer to the Navier Stokes module for more advanced
fluid flow capabilities.

!listing test/tests/fvkernels/fv_euler/fv_euler.i block=FVKernels

!syntax parameters /FVKernels/FVMatAdvection

!syntax inputs /FVKernels/FVMatAdvection

!syntax children /FVKernels/FVMatAdvection

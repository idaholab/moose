# Physics Modules

MOOSE includes a set of community developed physics modules that you can build on to create your own application.

* [Phase Field](modules/phase_field/index.md)
* [Tensor Mechanics](modules/tensor_mechanics/index.md)
* [Reconstructed Discontinous Galerkin](modules/rdg/index.md)
* [Navier-Stokes](modules/navier_stokes/index.md)
* [Level Set](modules/level_set/index.md)

The purpose of the modules is to encapsulate common kernels, boundary conditions, etc. to prevent code duplication.
Examples include: heat conduction, solid mechanics, Navier-Stokes, and others. The modules are organized so that your
application can link against only those which it requires.

!!! warning
    _No_ export controlled physics (e.g., neutronics) should be put into the modules.

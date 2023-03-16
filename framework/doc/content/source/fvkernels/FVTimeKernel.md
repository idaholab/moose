# FVTimeKernel

!syntax description /FVKernels/FVTimeKernel

The time derivative is automatically computed based on the
[time integration scheme selected](syntax/Executioner/TimeIntegrator/index.md). This
class should be used in finite volume simulations which leverage the
quadrature-point pre-initialized paradigm, which includes fully compressible
Navier-Stokes simulations.

!alert note
When creating a new time derivative kernel, developers should consider inheriting this class
as its provides the matrix/vector time tags. If not, those should be added in the `validParams()`
routine of the new class.

## Example input syntax

In this example, the
[Burger's equation](https://en.wikipedia.org/wiki/Burgers%27_equation) is solved
in one dimension. The time derivative term of the equation is added to the numerical system using
a `FVTimeKernel`.

!listing test/tests/fvkernels/fv_burgers/fv_burgers.i block=FVKernels

!syntax parameters /FVKernels/FVTimeKernel

!syntax inputs /FVKernels/FVTimeKernel

!syntax children /FVKernels/FVTimeKernel

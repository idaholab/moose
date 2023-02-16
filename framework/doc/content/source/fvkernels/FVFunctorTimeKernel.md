# FVFunctorTimeKernel

!syntax description /FVKernels/FVFunctorTimeKernel

The user may provide the `functor` parameter from which to query the time
derivative. If the `functor` parameter is not provided, then the variable that
this kernel acts on will be the functor used. The time derivative is
automatically computed for nonlinear and auxiliary variables based on the
[time integration scheme selected](syntax/Executioner/TimeIntegrator/index.md). Time
derivatives of `Function/ADFunction` functors are computed using those objects
`timeDerivative` APIs. Time derivatives of functor material properties are not
yet implemented. This class should be used in finite volume simulations which
leverage the
[on-the-fly functor evaluation system](Materials/index.md#functor-props), which
includes incompressible and weakly compressible Navier-Stokes simulations.

!alert note
When creating a new time derivative kernel, developers should consider inheriting this class
as it provides the matrix/vector time tags. If not, those should be added in the `validParams()`
routine of the new class.

## Example input syntax

In this example, the variable `v` is the solution of a simple time-dependent diffusion
problem. The time derivative term of the equation is added to the numerical system using
a `FVFunctorTimeKernel`.

!listing test/tests/fvkernels/fv_simple_diffusion/transient.i block=FVKernels

!syntax parameters /FVKernels/FVFunctorTimeKernel

!syntax inputs /FVKernels/FVFunctorTimeKernel

!syntax children /FVKernels/FVFunctorTimeKernel

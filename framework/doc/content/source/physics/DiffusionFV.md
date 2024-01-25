# DiffusionFV

!syntax description /Physics/Diffusion/FiniteVolume/DiffusionFV

See the [DiffusionPhysicsBase.md] documentation for the diffusion equation solved.

## Finite Volume Discretization

The volumetric discretization of the time derivative uses the [FVTimeKernel.md] kernel.
The diffusion term $\nabla \cdot D \nabla u(\vec{x})$ is integrated by parts and represented using a [FVDiffusion.md] kernel.
The source term $f$ is added using:

- a [FVBodyForce.md] if the source is a constant, a [Postprocessor](syntax/Postprocessors/index.md) or a [Function](syntax/Functions/index.md)
- a [FVCoupledForce.md] if the source is another type of [functor](syntax/Functors/index.md)


The Dirichlet boundary conditions are created using:

- a [FVDirichletBC.md] if the boundary value is set to a number
- a [FVFunctionDirichletBC.md] if set to a [Function](syntax/Functions/index.md)
- a [FVFunctorDirichletBC.md] for any other kind of [functor](syntax/Functors/index.md) for the boundary value


The Neumann boundary conditions are created using:

- a [FVNeumannBC.md] if the flux is set to a number
- a [FVFunctionNeumannBC.md] if set to a [Function](syntax/Functions/index.md)
- a [FVFunctorNeumannBC.md] for any other kind of [functor](syntax/Functors/index.md) for the flux value


!alert note
We could use a [Functor](syntax/Functors/index.md) object to cover every need, but the specialized objects
are a few percent faster, depending on the case.

!syntax parameters /Physics/Diffusion/FiniteVolume/DiffusionFV

!syntax inputs /Physics/Diffusion/FiniteVolume/DiffusionFV

!syntax children /Physics/Diffusion/FiniteVolume/DiffusionFV

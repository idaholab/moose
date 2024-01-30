# DiffusionCG

!syntax description /Physics/Diffusion/ContinuousGalerkin/DiffusionCG

See the [DiffusionPhysicsBase.md] documentation for the diffusion equation solved.

## Continuous Galerkin Discretization

The volumetric discretization of the time derivative uses the [TimeDerivative.md] kernel.
The diffusion term $\nabla \cdot D \nabla u(\vec{x})$ is integrated by parts and represented using:

- a [Diffusion.md] kernel if no diffusion coefficient is passed
- a [MatDiffusion.md] kernel if a diffusion coefficient is passed, usually as a [material property](syntax/Materials/index.md)


The source term $f$ is added using:

- a [BodyForce.md] if the source is a constant, a [Postprocessor](syntax/Postprocessors/index.md) or a [Function](syntax/Functions/index.md)
- a [MatCoupledForce.md] if the force is specified using a material property
- a [CoupledForce.md] if the source is another type of [functor](syntax/Functors/index.md)


The Dirichlet boundary conditions are created using:

- a [DirichletBC.md] if the boundary value is set to a number
- a [FunctionDirichletBC.md] if set to a [Function](syntax/Functions/index.md)
- a [PostprocessorDirichletBC.md] if set to a [Postprocessor](syntax/Postprocessors/index.md)
- a [FunctorDirichletBC.md] for any other kind of [functor](syntax/Functors/index.md) for the boundary value


The Neumann boundary conditions are created using:

- a [NeumannBC.md] if the flux is set to a number
- a [FunctionNeumannBC.md] if set to a [Function](syntax/Functions/index.md)
- a [PostprocessorNeumannBC.md] if set to a [Postprocessor](syntax/Postprocessors/index.md)
- a [FunctorNeumannBC.md] for any other kind of [functor](syntax/Functors/index.md) for the flux value


!alert note
We could use a [Functor](syntax/Functors/index.md) object to cover every need, but the specialized objects
are a few percent faster, depending on the case.

!alert note
The user may switch between using / not using automatic differentiation for the kernel and boundary
conditions using the [!param](/Physics/Diffusion/ContinuousGalerkin/use_automatic_differentiation). This parameter is only
obeyed if the AD/non-AD object exists for the requested diffusivity / source / boundary value functor etc.


!syntax parameters /Physics/Diffusion/ContinuousGalerkin/DiffusionCG

!syntax inputs /Physics/Diffusion/ContinuousGalerkin/DiffusionCG

!syntax children /Physics/Diffusion/ContinuousGalerkin/DiffusionCG

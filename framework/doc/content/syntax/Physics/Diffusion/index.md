# Diffusion Physics syntax

The `[Diffusion]` block is a sub-block of the `[Physics]` block.
`DiffusionPhysicsBase`-derived classes, such as [DiffusionCG.md]
can be created, nested under the relevant discretization sub-block.

For example, a [DiffusionCG.md] can be created inside the [Physics/Diffusion/ContinuousGalerkin](Physics/Diffusion/ContinuousGalerkin/index.md) block.

!listing test/tests/physics/diffusion_cg.i block=Physics

!syntax list /Physics/Diffusion objects=True actions=False subsystems=False

!syntax list /Physics/Diffusion objects=False actions=False subsystems=True

!syntax list /Physics/Diffusion objects=False actions=True subsystems=False

# HeatConduction Physics syntax

The `[HeatConduction]` block is a sub-block of the `[Physics]` block.
`HeatConductionPhysicsBase`-derived classes, such as [HeatConductionCG.md]
can be created, nested under the relevant discretization sub-block.

For example, a [continuous Galerkin finite element discretization of the heat conduction equation](HeatConductionCG.md) can be created inside the [Physics/HeatConduction/FiniteElement](Physics/HeatConduction/FiniteElement/index.md) block.

!listing test/tests/physics/test_cg.i block=Physics

!syntax list /Physics/HeatConduction objects=True actions=False subsystems=False

!syntax list /Physics/HeatConduction objects=False actions=False subsystems=True

!syntax list /Physics/HeatConduction objects=False actions=True subsystems=False

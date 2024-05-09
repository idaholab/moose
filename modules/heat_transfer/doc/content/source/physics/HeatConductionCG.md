# HeatConductionCG

This [Physics](syntax/Physics/index.md) object implements the heat conduction equation over a
volumetric domain using a continuous Galerkin finite element discretization.

It creates the kernels for each term of the equation:

- the conduction term using [ADHeatConduction.md]
- the time derivative of the energy term using [ADHeatConductionTimeDerivative.md] if in a transient solve
- the heat source term using [ADCoupledForce](CoupledForce.md) if specified

The boundary conditions are created with:

- a [FunctorNeumannBC.md] for heat flux boundary conditions
- a [FunctorDirichletBC.md] for fixed temperature boundary conditions

A boundary condition object is created for each boundary, except if the same arguments
can be used across all boundaries in which case a single object is created and restricted to
the union of boundaries.

!syntax parameters /Physics/HeatConduction/FiniteElement/HeatConductionCG

!syntax inputs /Physics/HeatConduction/FiniteElement/HeatConductionCG

!syntax children /Physics/HeatConduction/FiniteElement/HeatConductionCG

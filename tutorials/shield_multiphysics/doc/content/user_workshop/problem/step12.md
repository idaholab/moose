# Step 12: Custom Syntax id=step12

!---

Add custom syntax to build objects that are common to all conjugate heat transfer thermal mechanical problems:

- Kernels and BCs for solid heat conduction
- Objects for solid mechanics
- Kernels and BCs for fluid flow
- Kernels and BCs for fluid heat transfer

!!end-intro

!---

## Step 12: Heat Conduction Physics

Defines the kernels, the boundary conditions of selected supported types,
and some numerical parameters

[Physics/HeatConduction/FiniteElement](Physics/HeatConduction/FiniteElement/index.md)

!listing step12_physics/step12.i block=Physics/HeatConduction

!---

## Step 12: Solid Mechanics Physics

Defines the kernels and some helper materials for computing strain.

[Physics/SolidMechanics/QuasiStatic](Physics/SolidMechanics/QuasiStatic/index.md)

!listing step12_physics/step12.i block=Physics/SolidMechanics

!---

## Step 12: Fluid Flow Physics

The flow equations are separated from the conservation of energy in the fluid
for readability purposes.

Defines both the kernels and the boundary conditions.

[Physics/NavierStokes/Flow](Physics/NavierStokes/Flow/index.md)

!listing step12_physics/step12.i block=Physics/NavierStokes/Flow

!---

## Step 12: Fluid Energy conservation Physics

[Physics/NavierStokes/FluidHeatTransfer](Physics/NavierStokes/FluidHeatTransfer/index.md)

!listing step12_physics/step12.i block=Physics/NavierStokes/FluidHeatTransfer

!---

## Step 12: Setting up a multi-system simulation

First declare the names of the systems to use in the [Problem](syntax/Problem/index.md)

!listing step12_physics/step12.i block=Problem

then specify those systems in each `Physics`.

!---

## Step 12: Preconditioning

Multi-system allows for customization of solvers via [Preconditioning](Preconditioning/index.md).

[Single Matrix Preconditioner (SMP)](SingleMatrixPreconditioner.md)

!listing step12_physics/step12.i block=Preconditioning

!---

## Step 12: Run

```bash
cd ~/projects/moose/tutorials/shield_multiphysics/inputs/step12_physics
moose-opt -i step12.i
```

!---

## Step 12: Results

!style halign=center
!media shield_multiphysics/results/step12.png style=width:80%


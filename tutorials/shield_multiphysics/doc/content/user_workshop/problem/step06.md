# Step 6: Transient Heat Conduction id=step06

!! end-intro

To create a time-dependent problem add in the time derivative:

!equation
\rho c_p \frac{\partial T}{\partial t} - \nabla \cdot k \nabla T = 0

The time term exists in the heat transfer module as [HeatConductionTimeDerivative](/HeatConductionTimeDerivative.md).

!---

## Step 6: Time-dependent Input File

We introduce a time derivative kernel to model the contribution to the residual/Jacobian (and various time integration vectors) of the time derivative.

!listing step06_transient_heat_conduction/step6_transient.i block=Kernels

!---

Time stepping parameters are passed to the Executioner block with the [Transient](Transient.md) executioner type.

!listing step06_transient_heat_conduction/step6_transient.i block=Executioner

!---

## Step 6: Transient Simulation

There are generally two uses for time-dependent runs:

1. Transient: Performing a natural evolution of the model through time.
2. Pseudo-transient: Form of under-relaxation for a steady-state solve that more stably converges to the solution.

!---

Material properties now include specific heat capacity and density for the time-derivative term.

- [HeatConductionMaterial](HeatConductionMaterial.md) has a parameter for the heat capacity.
- [GenericConstantMaterial](GenericConstantMaterial.md) is used to define the density.

!listing step06_transient_heat_conduction/step6_transient.i
         diff=step04_heat_conduction/step4.i
         block=Materials

!---

## Step 6: Running Transient Simulation

```bash
cd ~/projects/moose/tutorials/shield_multiphysics/inputs/step06_transient_heat_conduction
moose-opt -i step6_transient.i
```

!---

## Step 6: Transient Simulation Result

!media shield_multiphysics/results/step06_transient.mp4


!---


## Step 6: Pseudo-Transient Simulation

Since the goal of a pseudo-transient is to achieve a steady state, the heat
capacity (and density) do not matter and simply serve as relaxation factors. So
in this example, we use a multiplier to reduce heat capacity to converge the
steady-state solution faster:

```text
cp_multiplier = 1e-6
```

!listing step06_transient_heat_conduction/step6_pseudo_transient.i
         diff=step06_transient_heat_conduction/step6_transient.i
         block=Materials


!---

The `Transient` executioner has a `steady_state_detection` option to determine if the simulation has reached a steady-state:

!listing step06_transient_heat_conduction/step6_pseudo_transient.i
         diff=step06_transient_heat_conduction/step6_transient.i
         block=Executioner

!---

## Step 6: Running Pseudo-Transient Simulation

```bash
cd ~/projects/moose/tutorials/shield_multiphysics/inputs/step06_transient_heat_conduction
moose-opt -i step6_pseudo_transient.i
```

!---

## Step 6: Pseudo-Transient Simulation Result

!media shield_multiphysics/results/step06_pseudo_transient.mp4

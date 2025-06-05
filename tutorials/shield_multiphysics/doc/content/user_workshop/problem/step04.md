# Step 4: Heat Conduction kernel with Material id=step04

!---

Instead of passing constant coefficients to the heat conduction kernel, we use the Material system to supply
the values.

!equation
-\nabla \cdot k \nabla T = 0,

where $k$ is the thermal conductivity.

This system allows for properties that vary in space and time, that can be coupled to variables
in the simulation.

!!end-intro

!---

## HeatConductionMaterial

Three material properties must be produced for consumption by kernels of the heat conduction equation:

- thermal conductivity by the conduction term
- density and specific heat by the time derivative of the energy

Both shall be computed with a single [Material](Materials/index.md) object: [HeatConductionMaterial](HeatConductionMaterial.md).

!style fontsize=60%
!listing step04_heat_conduction/step4.i block=Materials max-height=300px

!---

## HeatConduction Kernel

The [CoefDiffusion](CoefDiffusion.md) Kernel object uses input parameters for defining the thermal
conductivity. Instead, the [HeatConduction](HeatConduction.md) Kernel utilizes the material
properties defined in [HeatConductionMaterial](HeatConductionMaterial.md) automatically.

!listing step04_heat_conduction/step4.i block=Kernels

Note: we use kernels with automatic differentiation (AD) for simplicity.

!---

## Step 4: Input File

!listing step04_heat_conduction/step4.i

!---

## Step 4: Run

```bash
cd ~/projects/moose/tutorials/shield_multiphysics/inputs/step04_heat_conduction
moose-opt -i step4.i
```

!---

## Step 4: Result

!media shield_multiphysics/results/step04.png

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

## HeatConduction Material

Three material properties must be produced for consumption by kernels of the heat conduction equation:

- thermal conductivity by the conduction term
- density and specific heat by the time derivative of the energy

Both shall be computed with a single `Material` object: `HeatConductionMaterial`.

!---

## HeatConductionMaterial

!listing step04_heat_conduction/step4.i block=Materials

!---

## HeatConduction Kernel

The `CoefDiffusion` Kernel object uses input parameters for defining the thermal conductivity.
We modify it to to consume the newly created material properties.

!---

## ADHeatConduction

!listing step04_heat_conduction/step4.i block=Kernels

!---

## Step 4: Input File

!listing step04_heat_conduction/step4.i

!---

## Step 4: Run

```bash
cd ~/projects/moose/tutorials/shield_multiphysics/step04_heat_conduction
../executable/shield_multiphysics-opt -i step4.i
```

Using a prebuilt MOOSE from conda:

```bash
conda activate moose
cd ~/projects/moose/tutorials/shield_multiphysics/step04_heat_conduction
moose-opt -i step2.i
```

!---

## Step 4: Result

!media shield_multiphysics/results/step04.png style=width:70%;margin-left:auto;margin-right:auto

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

## HeatConductionMaterial.h

!listing step04_heat_conduction/include/materials/HeatConductionMaterial.h

!---

## HeatConductionMaterial.C

!listing step04_heat_conduction/src/materials/HeatConductionMaterial.C

!---

## HeatConduction Kernel

The `CoefDiffusion` Kernel object uses input parameters for defining the thermal conductivity.
We modify it to to consume the newly created material properties.

!---

## ADHeatConduction.h

!listing step04_heat_conduction/include/kernels/ADHeatConduction.h

!---

## ADHeatConduction.C

!listing step04_heat_conduction/src/kernels/ADHeatConduction.C

!---

## Step 4: Input File

!listing step04_heat_conduction/inputs/step4.i

!---

## Step 4: Run

```bash
cd ~/projects/moose/tutorials/shield_multiphysics/step04_heat_conduction
make -j 12 # use number of processors for your system
cd inputs
../moose-opt -i step4.i
```

!---

## Step 4: Result

!media shield_multiphysics/results/step04.png style=width:70%;margin-left:auto;margin-right:auto

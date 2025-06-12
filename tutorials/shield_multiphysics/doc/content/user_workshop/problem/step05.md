# Step 5: Auxiliary Variables id=step05

!!end-intro

!---

## Example use: creating a constant field

We are not solving for the temperature in the water yet. To represent it as a variable that is
not in the nonlinear system, we use an [auxiliary variable](AuxVariables/index.md).

!listing step05_auxiliary_variables/step5.i block=AuxVariables/T_fluid

!---

## Example use: postprocessing

Heat flux

The heat flux can be computed using Fourier's law:

!equation
q = - k \nabla T

!---

## DiffusionFlux AuxKernel

The primary unknown ("nonlinear variable") is the temperature.

Once the temperature is computed, the [AuxiliarySystem](AuxKernels/index.md) can compute and output the heat flux field using
the coupled temperature variable and the thermal conductivity property via [DiffusionFluxAux](DiffusionFluxAux.md).

Auxiliary variables come in two flavors: Nodal and Elemental.

Nodal auxiliary variables cannot couple to gradients of nonlinear variables since gradients of $C_0$
continuous variables are not well-defined at the nodes.

Elemental auxiliary variables can couple to gradients of nonlinear variables since evaluation
occurs in the element interiors.

!---

## Step 5: Input File

!listing step05_auxiliary_variables/step5.i
         diff=step04_heat_conduction/step4.i

!---

## Step 5: Run

```bash
cd ~/projects/moose/tutorials/shield_multiphysics/inputs/step05_auxiliary_variables
moose-opt -i step5.i
```

!---

## Step 5: Result

!media shield_multiphysics/results/step05.png

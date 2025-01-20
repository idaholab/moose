# Step 5: Auxiliary Variables id=step05

!!end-intro

!---

## Example use: creating a constant field

We are not solving for the temperature in the water yet. To represent it as a variable that is
not in the nonlinear system, we use an auxiliary variable.

!listing step05_auxiliary_variables/step5.i block=AuxVariables

!---

## Example use: postprocessing

Heat flux

The heat flux can be computed using Fourier's law:

!equation
q = - k \nabla T

!---

## DiffusionFlux AuxKernel

The primary unknown ("nonlinear variable") is the temperature

Once the temperature is computed, the AuxiliarySystem can compute and output the heat flux field using
the coupled temperature variable and the thermal conductivity property.

Auxiliary variables come in two flavors: Nodal and Elemental.

Nodal auxiliary variables cannot couple to gradients of nonlinear variables since gradients of $C_0$
continuous variables are not well-defined at the nodes.

Elemental auxiliary variables can couple to gradients of nonlinear variables since evaluation
occurs in the element interiors.

!---

## DiffusionFluxAux.h

!listing step05_auxiliary_variables/include/auxkernels/DiffusionFluxAux.h

!---

## DiffusionFluxAux.C

!listing step05_auxiliary_variables/src/auxkernels/DiffusionFluxAux.C

!---

## Step 5: Input File

!listing step05_auxiliary_variables/step5.i

!---

## Step 5: Run

```bash
cd ~/projects/moose/tutorials/shield_multiphysics/step05_auxiliary_variables
make -j 12 # use number of processors for your system
cd inputs
../moose-opt -i step5.i
```

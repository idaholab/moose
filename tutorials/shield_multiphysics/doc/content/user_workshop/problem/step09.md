# Step 9: Postprocessors id=step09

!---

Aggregate values based on simulation data are useful for understanding the simulation as well
as defining coupling values across coupled equations.

There are two main systems for aggregating data: Postprocessors and VectorPostprocessors.

!!end-intro

!---

## Step 9: Input File

!listing step09_postprocessing/step9.i

!---

## Step 9: Run

```bash
cd ~/projects/moose/tutorials/shield_multiphysics/step09_postprocessing
make -j 12 # use number of processors for your system
cd inputs
../moose-opt -i step9.i
```

Then plot in python, excel, paraview etc

# Step 8: Postprocessors id=step08

!---

Aggregate values based on simulation data are useful for understanding the simulation as well
as defining coupling values across coupled equations.

There are two main systems for aggregating data: Postprocessors and VectorPostprocessors.

!!end-intro

!---

## Step 8: Input File

!listing step08_postprocessors/problems/step8.i

!---

## Step 8: Run

```bash
cd ~/projects/moose/tutorials/darcy-thermo_mech/step08_postprocessors
make -j 12 # use number of processors for your system
cd problems
../darcy_thermo_mech-opt -i step8.i
```

!---

## Step 8: Result

!media darcy_thermo_mech/step08_result.mp4

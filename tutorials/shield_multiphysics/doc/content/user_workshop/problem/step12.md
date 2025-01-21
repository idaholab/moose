# Step 12: Custom Syntax id=step12

!---

Add custom syntax to build objects that are common to all conjugate heat transfer thermal mechanical problems:

- kernels for heat conduction equation
- kernels for solid mechanics
- kernels for Navier Stokes equation

!!end-intro

!---

## Step 12: Input File

!listing step12_action/step12.i

!---

## Step 12: Run

```bash
cd ~/projects/moose/tutorials/shield_multiphysics/step12_action
make -j 12 # use number of processors for your system
cd inputs
../moose-opt -i step12.i
```


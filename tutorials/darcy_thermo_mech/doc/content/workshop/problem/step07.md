# Step 7: Mesh Adaptivity id=step07

!!end-intro

!---

## Step 7a: Coarse Solution

!listing step7a_coarse.i

!---

## Step 7a: Run

```bash
cd ~/projects/moose/tutorials/darcy-thermo_mech/step07_adaptivity
make -j 12 # use number of processors for your system
cd problems
../darcy_thermo_mech-opt -i step7a_coarse.i
```

!---

## Step 7b: Fine Solution

!listing step7b_fine.i

!---

## Step 7b: Run

```bash
cd ~/projects/moose/tutorials/darcy-thermo_mech/step07_adaptivity
make -j 12 # use number of processors for your system
cd problems
../darcy_thermo_mech-opt -i step7b_fine.i
```

!---

## Step 7c: Adaptive Mesh Solution

!listing step7c_adapt.i

!---

## Step 7c: Run

```bash
cd ~/projects/moose/tutorials/darcy-thermo_mech/step07_adaptivity
make -j 12 # use number of processors for your system
cd problems
../darcy_thermo_mech-opt -i step7a_adapt.i
```

!---

!media darcy_thermo_mech/step07abc_result.mp4

!---

## Step 7d: Multiple Subdomains

!listing step7d_adapt_blocks.i

!---

!media darcy_thermo_mech/step07d_result.mp4

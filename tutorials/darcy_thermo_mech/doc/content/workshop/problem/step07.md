# Step 7: Mesh Adaptivity id=step07

!!end-intro

!---

## Step 7a: Coarse Solution

!listing step07_adaptivity/problems/step7a_coarse.i diff=step06_coupled_darcy_heat_conduction/problems/step6a_coupled.i

!---

## Step 7a: Run

```bash
cd ~/projects/moose/tutorials/darcy_thermo_mech/step07_adaptivity
make -j 12 # use number of processors for your system
cd problems
../darcy_thermo_mech-opt -i step7a_coarse.i
```

!---

## Step 7b: Fine Solution

!listing step07_adaptivity/problems/step7b_fine.i diff=step07_adaptivity/problems/step7a_coarse.i

!---

## Step 7b: Run

```bash
cd ~/projects/moose/tutorials/darcy_thermo_mech/step07_adaptivity
make -j 12 # use number of processors for your system
cd problems
../darcy_thermo_mech-opt -i step7b_fine.i
```

!---

## Step 7c: Adaptive Mesh Solution

!listing step07_adaptivity/problems/step7c_adapt.i diff=step07_adaptivity/problems/step7b_fine.i

!---

## Step 7c: Run

```bash
cd ~/projects/moose/tutorials/darcy_thermo_mech/step07_adaptivity
make -j 12 # use number of processors for your system
cd problems
../darcy_thermo_mech-opt -i step7a_adapt.i
```

!---

!media darcy_thermo_mech/step07abc_result.mp4

!---

## Step 7d: Multiple Subdomains

!listing step7d_adapt_blocks.i diff=step07_adaptivity/problems/step7c_adapt.i

!---

!media darcy_thermo_mech/step07d_result.mp4

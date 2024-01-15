# Step 11: Multiscale Simulation id=step11

!---

We want to study the effects of the local environment on sensors placed anywhere in the shield.

!!end-intro

## Step 11: Lower-scale Input File

!listing step11_multiapps/inputs/step11_local.i

!---

## Step 11: Run lower-scale

Before setting up the multiapp and the transfers, we must make sure
each input file runs well separately.

```bash
cd ~/projects/moose/tutorials/shield_multiphysics/step11_multiapps
make -j 12 # use number of processors for your system
cd inputs
../moose-opt  -i step11_micro.i
```

!---

## Step 11: Multi-scale Input File

We distribute these simulations using a `MultiApp`.
The positions of each child app are specified using a `Positions` object.

!listing step11_multiapps/inputs/step11_global.i

!---

## Step 11: Run Multi-scale

```bash
cd ~/projects/moose/tutorials/shield_multiphysics/step11_multiapps
make -j 12 # use number of processors for your system
cd inputs
../moose-opt -i step11_global.i
```

# Step 11: Multiscale Simulation id=step11

!---

We want to study the effects of the local environment on sensors placed anywhere in the shield.

!media shield_sensor_spread.png
       style=width:80%;margin-left:auto;margin-right:auto;display:block;box-shadow:none;

!!end-intro

!---

## Step 11: Lower-scale Input File

!listing step11_multiapps/step11_local.i

!---

## Step 11: Run lower-scale

Before setting up the multiapp and the transfers, we must make sure
each input file runs well separately.

With the tutorial executable:

```bash
cd ~/projects/moose/tutorials/shield_multiphysics/step11_multiapps
../executable/shield_multiphysics-opt -i step11_local.i
```

With a conda MOOSE executable:

```bash
conda activate moose
cd ~/projects/moose/tutorials/shield_multiphysics/step11_multiapps
moose-opt -i step11_local.i
```

!---

!media results/step11_local.png caption="Sensor temperature field with dummy fixed boundary conditions"

!---

## Step 11: Multi-scale Input File

We distribute these simulations using a `MultiApp`.
The positions of each child app are specified using a `Positions` object.

!listing step11_multiapps/step11_global.i

!---

## Step 11: Run Multi-scale

With the tutorial executable:

```bash
cd ~/projects/moose/tutorials/shield_multiphysics/step11_multiapps
../executable/shield_multiphysics-opt -i step11_global.i
```

With a conda MOOSE executable:

```bash
conda activate moose
cd ~/projects/moose/tutorials/shield_multiphysics/step11_multiapps
moose-opt -i step11_global.i
```

!---

!media results/step11_global_distrib.png caption="The individual sensor results can be plotted 'in-position' in the global geometry"

!---

!media results/step11_global.png caption="Postprocessed quantities can be transferred from the local child app meshes to the global mesh"

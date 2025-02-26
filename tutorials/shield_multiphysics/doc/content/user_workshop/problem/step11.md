# Step 11: Multiscale Simulation id=step11

!---

We want to

1. Couple the solid heat conduction solve with the fluid solve and
2. Study the effects of the local environment on sensors placed in the shield.

!row!

!col! width=50%

!media shield_sensor_spread.png style=width:55%;margin-left:auto;margin-right:auto;display:block

!col-end!

!col! width=50%

!media sensor_xs.png

!col-end!

!row-end!

!!end-intro

!---

## Step 11: Fluid Input File

Primary difference is the addition of the `T_solid` auxiliary variable, which is
used as the "container" for solid heat-conduction app to transfer its solution
into.

!listing step11_multiapps/step11_2d_fluid.i
         diff=step10_finite_volume/step10.i

!---

## Step 11: Lower-scale Input File

!listing step11_multiapps/step11_local.i

!---

## Step 11: Run lower-scale

Before setting up the multiapp and the transfers, we must make sure
each input file runs well separately.

```bash
cd ~/projects/moose/tutorials/shield_multiphysics/step11_multiapps
moose-opt -i step11_local.i
```

!---

!media results/step11_local.png
       style=width:60%;margin-left:auto;margin-right:auto;display:block

!style halign=center
Sensor temperature field with dummy fixed boundary conditions

!---

## Step 11: Multi-scale Input File

We distribute these simulations using a `MultiApp`.
The positions of each child app are specified using a `Positions` object.

!listing step11_multiapps/step11_2d_heat_conduction.i

!---

## Step 11: Run Multi-scale

Run stand-alone heat-conduction to test input:

```bash
moose-opt -i step11_2d_heat_conduction.i MultiApps/active='' Transfers/active=''
```

Run full model:

```bash
moose-opt -i step11_2d_heat_conduction.i
```


!---

The individual sensor results can be plotted 'in-position' in the global geometry.

Postprocessed quantities can be transferred from the local child app meshes to the global mesh.

!media results/step11_global.png style=width:70%;margin-left:auto;margin-right:auto;display:block

# Step 11: Multiscale Simulation id=step11

!---

We want to

1. Couple the solid heat conduction solve with the fluid solve and
2. Study the effects of the local environment on sensors placed in the shield.

!row!

!col! width=50%

!media shield_sensor_spread.png
       style=width:55%;margin-left:auto;margin-right:auto;display:block
       alt=Diagram of the mesh, with sensors embedded in the ordinary concrete and aluminum layers.

!col-end!

!col! width=50%

!media sensor_xs.png
       alt=The mesh for spherical sensors, with boron and HDPE layers.

!col-end!

!row-end!

!!end-intro

!---

## Step 11: MultiApp Flow

Three inputs representing each region/physics:

1. Solid heat conduction: `step11_2d_heat_conduction.i`
2. Thermal fluids: `step11_2d_fluid.i`
3. Sensor response: `step11_local.i`

!media shield_multiphysics/results/step11_flow.png
       style=width:55%;margin-left:auto;margin-right:auto;display:block
       alt=Illustration of the exchange of temperature information between the three components of the simulation.

!---

## Step 11: Fluid Coupling

!row!

!col! width=40%

### Fluids Sub-App

Primary difference from the previous step is the addition of the `T_solid` auxiliary variable, which is
used as the "container" for solid heat-conduction app to transfer its solution
into.

!style fontsize=70% halign=left
!listing step11_multiapps/step11_2d_fluid.i
         block=AuxVariables FVBCs/T_fluid_water_boundary
         diff=step10_finite_volume/step10.i
         style=width:70%;height:280px

!col-end!

!col! width=16%

!! Intentional comment to provide extra spacing

[-](https://mooseframework.inl.gov/ style=color:white;)

!col-end!

!col! width=40%

### Heat Conduction Main App

- [TransientMultiApp](TransientMultiApp.md)
- [MultiAppCopyTransfer](MultiAppCopyTransfer.md)

!style fontsize=70% halign=right
!listing step11_multiapps/step11_2d_heat_conduction.i
         block=AuxVariables/T_fluid MultiApps/fluid Transfers/send_T_solid Transfers/recv_T_fluid
         style=width:70%;height:300px

!col-end!

!row-end!

!---

## Multi-Scale Applications

!row!

!col! width=40%

### Sensor Sub-App

!style fontsize=70% halign=left
!listing step11_multiapps/step11_local.i
         style=width:70%;height:350px

!col-end!

!col! width=17%

!! Intentional comment to provide extra spacing

[-](https://mooseframework.inl.gov/ style=color:white;)

!col-end!

!col! width=4%

### Heat Conduction Main App

!style! fontsize=80%

- [TransientMultiApp](TransientMultiApp.md)
- [MultiAppVariableValueSamplePostprocessorTransfer](MultiAppVariableValueSamplePostprocessorTransfer.md)
- [MultiAppVariableValueSampleTransfer](MultiAppVariableValueSampleTransfer.md)
- [MultiAppPostprocessorInterpolationTransfer](MultiAppPostprocessorInterpolationTransfer.md)

!style-end!

!style fontsize=70% halign=right
!listing step11_multiapps/step11_2d_heat_conduction.i
         block=AuxVariables/flux AuxVariables/T_hdpe_inner AuxVariables/T_boron_inner
               MultiApps/detectors
               Transfers/send_exterior_temperature Transfers/send_local_flux Transfers/hdpe_temperature Transfers/boron_temperature
         style=width:70%;height:200px

!col-end!

!row-end!

!---

## Step 11: Run Decoupled Physics

!row!

!col! width=45%

Before fully coupled simulation, we must make sure
each input file runs well separately.

!style fontsize=90%;width:80%
```bash
cd ~/projects/moose/tutorials/shield_multiphysics/inputs/step11_multiapps
moose-opt -i mesh2d_coarse.i --mesh-only
moose-opt -i step11_local.i
moose-opt -i step11_2d_fluid.i
# Disable multiapps and transfers to run the parent application standalone
moose-opt -i step11_2d_heat_conduction.i MultiApps/active='' Transfers/active=''
```

!col-end!

!col! width=45%

!media results/step11_local.png
       style=width:40%;margin-left:auto;margin-right:auto;display:block
       alt=Temperature field of the sensor, when run with decoupled physics

!style halign=center
Sensor temperature field with dummy fixed boundary conditions

!col-end!

!row-end!

!---

## Step 11: Run Multi-scale

```bash
moose-opt -i step11_2d_heat_conduction.i
```

!---

The individual sensor results can be plotted 'in-position' in the global geometry.

Postprocessed quantities can be transferred from the local child app meshes to the global mesh.

!media results/step11_global.png
       style=width:70%;margin-left:auto;margin-right:auto;display:block
       alt=Temperature of the fluid, solid walls, and sensors. Also plotted are the temperatures of the upper and lower boron liners.

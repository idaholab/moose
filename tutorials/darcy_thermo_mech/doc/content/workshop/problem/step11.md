# Step 11: Custom Syntax id=step11

!---

Add custom syntax to build objects that are common to all Darcy thermal mecahnical problems:

- Velocity auxiliary variables and kernels
- Pressure kernel
- Temperature kernels

!!end-intro

!---

## SetupDarcySimulation.h

!listing step11_action/include/actions/SetupDarcySimulation.h

!---

## SetupDarcySimulation.C

!listing step11_action/src/actions/SetupDarcySimulation.C

!---

## DarcyThermoMechApp.h

!listing step11_action/include/base/DarcyThermoMechApp.h

!---

## DarcyThermoMechApp.C

!listing step11_action/src/base/DarcyThermoMechApp.C

!---

## Step 11: Input File

!listing step11_action/problems/step11.i

!---

## Step 11: Run

```bash
cd ~/projects/moose/tutorials/darcy-thermo_mech/step11_action
make -j 12 # use number of processors for your system
cd problems
../darcy_thermo_mech-opt -i step11.i
```

!---

## Step 11: Results

!media darcy_thermo_mech/step09_result.mp4

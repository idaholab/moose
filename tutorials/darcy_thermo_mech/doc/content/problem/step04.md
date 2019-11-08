# Step 4: Improve DarcyPressure Kernel id=step04

!---

The `DarcyPressure` object is using AD, but it can be improved further by inheriting from
similar, existing code within MOOSE.

To compute the desired value, the residual of the `ADDiffusion` object simply needs a
multiplier.

!---

## DarcyPressure.h

!listing step04_ad_diff_darcy_pressure/include/kernels/DarcyPressure.h

!---

## DarcyPressure.C

!listing step04_ad_diff_darcy_pressure/src/kernels/DarcyPressure.C

!---

## Step 4: Input File

!listing step04_ad_diff_darcy_pressure/problems/step4.i

!---

## Step 4: Run and Visualize with Peacock

```bash
cd ~/projects/moose/tutorials/darcy-thermo_mech/step04_ad_diff_darcy_pressure
make -j 12 # use number of processors for you system
cd problems
~/projects/moose/python/peacock/peacock -i step4.i
```

!---

## Step 3: Run via Command-line

```bash
cd ~/projects/moose/tutorials/darcy-thermo_mech/step04_ad_diff_darcy_pressure
make -j 12 # use number of processors for you system
cd problems
../darcy_thermo_mech-opt -i step4.i
```

!---

## Step 3: Visualize Result

```bash
~/projects/moose/python/peacock/peacock -r step4_out.e
```

!media step02_result.png !!Results are the same, just use it again

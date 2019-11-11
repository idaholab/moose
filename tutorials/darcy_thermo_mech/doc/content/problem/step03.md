# Step 3: AD Pressure Kernel id=step03

!---

## ADKernel Object

To use automatic differentiation, the `DarcyPressure` object must be altered to inherit from
`ADKernel`.

!---

## DarcyPressure.h

!listing step03_ad_darcy_pressure/include/kernels/DarcyPressure.h

!---

## DarcyPressure.C

!listing step03_ad_darcy_pressure/src/kernels/DarcyPressure.C

!---

## Step 3: Input File

!listing step03_ad_darcy_pressure/problems/pressure.i

!---

## Step 3: Run and Visualize with Peacock

```bash
cd ~/projects/moose/tutorials/darcy-thermo_mech/step03_ad_darcy_pressure
make -j 12 # use number of processors for you system
cd problems
~/projects/moose/python/peacock/peacock -i pressure.i
```

!---

## Step 3: Run via Command-line

```bash
cd ~/projects/moose/tutorials/darcy-thermo_mech/step03_ad_darcy_pressure
make -j 12 # use number of processors for you system
cd problems
../darcy_thermo_mech-opt -i pressure.i
```

!---

## Step 3: Visualize Result

```bash
~/projects/moose/python/peacock/peacock -r pressure_out.e
```

!media step02_result.png !!Results are the same, just use it again

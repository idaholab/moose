# Step 6: Function Properties id=step06

!---

## Fluctuating Spheres

The sphere size in the domain may not be constant, thus this property can quickly be
changed to a `Function` that will allow it to be defined with respect to space and time.

!!end-intro

!---

## Fluctuating Spheres

Update `PackedColumn` object to utilize a function and then change the input file to vary the
sphere size from 1 to 3 along the length of the pipe.

!---

## PackedColumn.h

!listing step06_functions/include/materials/PackedColumn.h

!---

## PackedColumn.C

!listing step06_functions/src/materials/PackedColumn.C

!---

## Step 6: Input File

!listing step06_functions/problems/pressure.i

!---

## Step 6: Run and Visualize with Peacock

```bash
cd ~/projects/moose/tutorials/darcy-thermo_mech/step06_functions
make -j 12 # use number of processors for you system
cd problems
~/projects/moose/python/peacock/peacock -i pressure.i
```

!---

## Step 6: Run via Command-line

```bash
cd ~/projects/moose/tutorials/darcy-thermo_mech/step06_functions
make -j 12 # use number of processors for you system
cd problems
../darcy_thermo_mech-opt -i pressure.i
```

!---

## Step 6: Visualize Result

```bash
~/projects/moose/python/peacock/peacock -r pressure_out.e
```

!media step06_result.png

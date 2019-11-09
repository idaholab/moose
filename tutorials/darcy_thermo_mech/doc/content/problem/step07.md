# Step 7: Material with AD Properties id=step07

!---

## AD Material Properties

The permability is defined as a function of space and time; for some problems (e.g., solid mechanics)
the Jacobian will be dependant upon this function, due to displacement calculations.


!!end-intro

!---

## PackedColumn with ADMaterial

To compute derivatives automatically for material properties inherit from `ADMaterial` in similar
fashion to that of Kernel objects.

!---

## PackedColumn.h

!listing step07_ad_darcy_material/include/materials/PackedColumn.h

!---

## PackedColumn.C

!listing step07_ad_darcy_material/src/materials/PackedColumn.C

!---

## Step 7: Input File

!listing step07_ad_darcy_material/problems/step7.i

!---

## Step 7: Run and Visualize with Peacock

```bash
cd ~/projects/moose/tutorials/darcy-thermo_mech/step07_ad_darcy_material
make -j 12 # use number of processors for you system
cd problems
~/projects/moose/python/peacock/peacock -i step7.i
```

!---

## Step 7: Run via Command-line

```bash
cd ~/projects/moose/tutorials/darcy-thermo_mech/step07_ad_darcy_material
make -j 12 # use number of processors for you system
cd problems
../darcy_thermo_mech-opt -i step7.i
```

!---

## Step 7: Visualize Result

```bash
~/projects/moose/python/peacock/peacock -r step7_out.e
```

!media step07_result.png

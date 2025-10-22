# Step 3: Pressure Kernel with Material id=step03

!---

Instead of passing constant parameters to the DarcyPressure object use the Material system to supply
the values.

!equation
-\nabla \cdot \frac{\mathbf{K}}{\mu} \nabla p = 0,

where $\textbf{K}$ is the permeability tensor and $\mu$ is the fluid viscosity.

This system allows for properties that vary in space and time, that can be coupled to variables
in the simulation.

!!end-intro

!---

## PackedColumn Material

Two material properties must be produced for consumption by DarcyPressure object: permeability and
viscosity.

Both shall be computed with a single `Material` object: `PackedColumn`.

As in the reference article, permeability varies with the size of the steel spheres, so we'll
perform an interpolation calculation for it over the range of valid values.

!---

## PackedColumn.h

!listing step03_darcy_material/include/materials/PackedColumn.h

!---

## PackedColumn.C

!listing step03_darcy_material/src/materials/PackedColumn.C

!---

## DarcyPressure Kernel

The existing Kernel object uses input parameters for defining permeability and viscosity,
it must be updated to consume the newly created material properties.

!---

## DarcyPressure.h

!listing step03_darcy_material/include/kernels/DarcyPressure.h diff=step02_darcy_pressure/include/kernels/DarcyPressure.h

!---

## DarcyPressure.C

!listing step03_darcy_material/src/kernels/DarcyPressure.C diff=step02_darcy_pressure/src/kernels/DarcyPressure.C

!---

## Step 3: Input File

!listing step03_darcy_material/problems/step3.i diff=step02_darcy_pressure/problems/step2.i

!---

## Step 3: Run

```bash
cd ~/projects/moose/tutorials/darcy_thermo_mech/step03_darcy_material
make -j 12 # use number of processors for your system
cd problems
../darcy_thermo_mech-opt -i step3.i
```

!---

## Step 3: Result

!media darcy_thermo_mech/step03_result.png
       alt=Pressure field obtained by running the simulation above.

!---

## Step 3b: Variable Spheres

Update the input file to vary the sphere size from 1 to 3 along the length of the pipe.

!---

## Step 3b: Input File

!listing step03_darcy_material/problems/step3b.i diff=step03_darcy_material/problems/step3.i

!---

## Step 3b: Result

!media darcy_thermo_mech/step03b_result.png
       alt=Pressure and permeability fields for variable-sized spheres, obtained by running the simulation above.

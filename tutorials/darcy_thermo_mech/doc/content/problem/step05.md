# Step 5: Pressure Kernel with Material id=step05

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

As in the reference article, permeability varies with the size of the steel spheres, so linear
interpolation will be used for defining this property.

!---

## PackedColumn.h

!listing step05_darcy_material/include/materials/PackedColumn.h

!---

## PackedColumn.C

!listing step05_darcy_material/src/materials/PackedColumn.C

!---

## DarcyPressure Kernel

The existing Kernel object uses input parameters for defining permeability and viscosity,
it must be updated to consume the newly created material properties.

!---

## DarcyPressure.h

!listing step05_darcy_material/include/kernels/DarcyPressure.h

!---

## DarcyPressure.C

!listing step05_darcy_material/src/kernels/DarcyPressure.C

!---

## Step 5: Input File

!listing step05_darcy_material/problems/step5.i

!---

## Step 5: Run and Visualize with Peacock

```bash
cd ~/projects/moose/tutorials/darcy-thermo_mech/step05_darcy_material
make -j 12 # use number of processors for you system
cd problems
~/projects/moose/python/peacock/peacock -i step5.i
```

!---

## Step 5: Run via Command-line

```bash
cd ~/projects/moose/tutorials/darcy-thermo_mech/step05_darcy_material
make -j 12 # use number of processors for you system
cd problems
../darcy_thermo_mech-opt -i step5.i
```

!---

## Step 5: Visualize Result

```bash
~/projects/moose/python/peacock/peacock -r step5_out.e
```

!media step05_result.png

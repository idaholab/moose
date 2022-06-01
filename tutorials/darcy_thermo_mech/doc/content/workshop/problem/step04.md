# Step 4: Velocity Auxiliary Variable id=step04

!---

The velocity is the primary variable of interest, it is computed base on the pressure as:

!equation
\vec{u} = -\frac{\mathbf{K}}{\mu} \nabla p

!!end-intro

!---

## DarcyVelocity AuxKernel

The primary unknown ("nonlinear variable") is the pressure

Once the pressure is computed, the AuxiliarySystem can compute and output the velocity field using
the coupled pressure variable and the permeability and viscosity properties.

Auxiliary variables come in two flavors: Nodal and Elemental.

Nodal auxiliary variables cannot couple to gradients of nonlinear variables since gradients of $C_0$
continuous variables are not well-defined at the nodes.

Elemental auxiliary variables can couple to gradients of nonlinear variables since evaluation
occurs in the element interiors.

!---

## DarcyVelocity.h

!listing step04_velocity_aux/include/auxkernels/DarcyVelocity.h

!---

## DarcyVelocity.C

!listing step04_velocity_aux/src/auxkernels/DarcyVelocity.C

!---

## Step 4: Input File

!listing step04_velocity_aux/problems/step4.i

!---

## Step 4: Run and Visualize with Peacock

```bash
cd ~/projects/moose/tutorials/darcy-thermo_mech/step04_velocity_aux
make -j 12 # use number of processors for your system
cd problems
~/projects/moose/python/peacock/peacock -i step4.i
```

!---

## Step 4: Run via Command-line

```bash
cd ~/projects/moose/tutorials/darcy-thermo_mech/step04_velocity_aux
make -j 12 # use number of processors for you system
cd problems
../darcy_thermo_mech-opt -i step3.i
```

!---

## Step 4: Visualize Result

```bash
~/projects/moose/python/peacock/peacock -r step4_out.e
```

!media darcy_thermo_mech/step04_result.png

!---

## Tighter Solve Tolerance

```bash
cd ~/projects/moose/tutorials/darcy-thermo_mech/step04_velocity_aux
make -j 12 # use number of processors for your system
cd problems
../darcy_thermo_mech-opt -i step4.i Executioner/nl_rel_tol=1e-12
```

!media darcy_thermo_mech/step04_result_tight.png

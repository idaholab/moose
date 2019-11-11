# Step 8: Velocity Auxiliary Variable id=step04

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

!listing step08_velocity_aux/include/auxkernels/DarcyVelocity.h

!---

## DarcyVelocity.C

!listing step08_velocity_aux/src/auxkernels/DarcyVelocity.C

!---

## Step 8: Input File

!listing step08_velocity_aux/problems/velocity.i

!---

## Step 8: Run and Visualize with Peacock

```bash
cd ~/projects/moose/tutorials/darcy-thermo_mech/step08_velocity_aux
make -j 12 # use number of processors for you system
cd problems
~/projects/moose/python/peacock/peacock -i velocity.i
```

!---

## Step 8: Run via Command-line

```bash
cd ~/projects/moose/tutorials/darcy-thermo_mech/step08_velocity_aux
make -j 12 # use number of processors for you system
cd problems
../darcy_thermo_mech-opt -i velocity.i
```

!---

## Step 8: Visualize Result

```bash
~/projects/moose/python/peacock/peacock -r velocity_out.e
```

!media step08_result.png

!---

## Tighter Solve Tolerance

```bash
cd ~/projects/moose/tutorials/darcy-thermo_mech/step08_velocity_aux
make -j 12 # use number of processors for you system
cd problems
../darcy_thermo_mech-opt -i velocity.i Executioner/nl_rel_tol=1e-12
```

!media step08_result_tight.png

# Step 9: Heat Conduction id=step09

!---

With the pressure equation handled, the heat conduction equation is next.

!equation
C\left( \frac{\partial T}{\partial t} + \epsilon \vec{u}\cdot\nabla T \right) - \nabla\cdot k \nabla T = 0

!---

Initially, only the steady heat conduction equation is considered:

!equation
-\nabla \cdot k \nabla T = 0

This is another diffusion-type term that depends on the thermal conductivity, $k$. This term is
implemented in the MOOSE heat conduction module as `ADHeatConduction`.

!---

## ADHeatConduction.h

!listing heat_conduction/include/kernels/ADHeatConduction.h

!---

## ADHeatConduction.C

!listing heat_conduction/src/kernels/ADHeatConduction.C

!---

The ADHeatCondution Kernel in conjunction with a `GenericConstantMaterial` is all that is needed
to perform a steady state heat conduction solve (with $T=350$ at the inlet and $T=300$ at the
outlet).

!---

## GenericConstantMaterial

GenericConstantMaterial is a simple way to define constant material properties.

Two input parameters are provided using "list" syntax common to MOOSE:

```text
prop_names  = 'conductivity density'
prop_values = '0.01         200'
```

!---

## Step 9a: Steady-State Input File

!listing step09_heat_conduction/problems/heat_steady.i

!---

## Step 9a: Running Input File

```bash
cd ~/projects/moose/tutorials/darcy-thermo_mech/step09_heat_conduction
make -j 12 # use number of processors for you system
cd problems
../darcy_thermo_mech-opt -i heat_steady.i
```

!!end-steady

!---

## Transient Heat Conduction (9b)

To create a time-dependent problem add in the time derivative:

!equation
C \frac{\partial T}{\partial t} - \nabla \cdot k \nabla T = 0

The time term exists in the heat conduction module as `ADHeatConductionTimeDerivative`, thus
only an update to the input file is required to run the transient case.

!---

## ADHeatConductionTimeDerivative.h

!listing heat_conduction/include/kernels/ADHeatConductionTimeDerivative.h

!---

## ADHeatConductionTimeDerivative.C

!listing heat_conduction/src/kernels/ADHeatConductionTimeDerivative.C

!---

## Step 9b: Time-dependent Input File

!listing step09_heat_conduction/problems/heat_transient.i

!---

## Step 9b: Running Input File

```bash
cd ~/projects/moose/tutorials/darcy-thermo_mech/step09_heat_conduction
make -j 12 # use number of processors for you system
cd problems
../darcy_thermo_mech-opt -i heat_transient.i
```

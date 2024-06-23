# Step 5: Heat Conduction id=step05

!---

With the pressure equation handled, the heat conduction equation is next.

!equation
\rho c_p \left( \frac{\partial T}{\partial t} + \epsilon \vec{u}\cdot\nabla T \right) - \nabla\cdot k \nabla T = 0

!---

Initially, only the steady heat conduction equation is considered:

!equation
-\nabla \cdot k \nabla T = 0

This is another diffusion-type term that depends on the thermal conductivity, $k$. This term is
implemented in the MOOSE heat transfer module as `ADHeatConduction`.

!---

## ADHeatConduction.h

!listing heat_transfer/include/kernels/ADHeatConduction.h

!---

## ADHeatConduction.C

!listing heat_transfer/src/kernels/ADHeatConduction.C

!---

The ADHeatConduction Kernel in conjunction with a `GenericConstantMaterial` is all that is needed
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

## Step 5a: Steady-State Input File

!listing step05_heat_conduction/problems/step5a_steady.i

!---

## Step 5a: Running Input File

```bash
cd ~/projects/moose/tutorials/darcy_thermo_mech/step5_heat_conduction
make -j 12 # use number of processors for your system
cd problems
../darcy_thermo_mech-opt -i step5a_steady.i
```

!!start-transient

!---

## Transient Heat Conduction (5b)

To create a time-dependent problem add in the time derivative:

!equation
\rho c_p \frac{\partial T}{\partial t} - \nabla \cdot k \nabla T = 0

The time term exists in the heat transfer module as `ADHeatConductionTimeDerivative`, thus
only an update to the input file is required to run the transient case.

!!end-steady

!---

## ADHeatConductionTimeDerivative.h

!listing heat_transfer/include/kernels/ADHeatConductionTimeDerivative.h

!---

## ADHeatConductionTimeDerivative.C

!listing heat_transfer/src/kernels/ADHeatConductionTimeDerivative.C

!---

## Step 5b: Time-dependent Input File

!listing step05_heat_conduction/problems/step5b_transient.i

!---

## Step 5b: Running Input File

```bash
cd ~/projects/moose/tutorials/darcy_thermo_mech/step5_heat_conduction
make -j 12 # use number of processors for your system
cd problems
../darcy_thermo_mech-opt -i step5b_transient.i
```

!!start-bc

!---

## Outflow Boundary Condition (5c)

The flow is assumed to exit the pipe into a large tank, which is modeled with the "No BC" boundary
condition of [!cite](griffiths1997no).

The boundary term, $-\left < k \nabla T \cdot \mathbf{n}, \psi_i \right >$, is computed implicitly
rather than being replaced with a known flux, as is done in a `NeumannBC`.

!!end-transient


!---

## HeatConductionOutflow.h

!listing step05_heat_conduction/include/bcs/HeatConductionOutflow.h

!---

## HeatConductionOutflow.C

!listing step05_heat_conduction/src/bcs/HeatConductionOutflow.C

!---

## Step5c: Outflow Input File

!listing step05_heat_conduction/problems/step5c_outflow.i

!---

## Step 5c: Run

```bash
cd ~/projects/moose/tutorials/darcy_thermo_mech/step05_heat_conduction
make -j 12 # use number of processors for your system
cd problems
../darcy_thermo_mech-opt -i step5b_transient.i
```

!---

## Step 5c: Results

!media darcy_thermo_mech/step05c_result.webm

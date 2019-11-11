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

## Step 5c: Run and Visualize with Peacock

```bash
cd ~/projects/moose/tutorials/darcy-thermo_mech/step05_heat_conduction
make -j 12 # use number of processors for you system
cd problems
~/projects/moose/python/peacock/peacock -i step5b_transient.i
```

!---

## Step 5c: Results

!media step05c_result.webm

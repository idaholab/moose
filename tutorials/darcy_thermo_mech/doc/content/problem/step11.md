# Step 11: Outflow Boundary Condition id=step11

The flow is assumed to exit the pipe into a large tank, which is modeled with the "No BC" boundary
condition of [!cite](griffiths1997no).

The boundary term, $-\left < k \nabla T \cdot \mathbf{n}, \psi_i \right >$, is computed implicitly
rather than being replaced with a known flux, as is done in a `NeumannBC`.

!!end-intro


!---

## HeatConductionOutflow.h

!listing step11_boundary_conditions/include/bcs/HeatConductionOutflow.h

!---

## HeatConductionOutflow.C

!listing step11_boundary_conditions/src/bcs/HeatConductionOutflow.C

!---

## Step 11: Outflow Input File

!listing step11_boundary_conditions/problems/heat_transient.i

!---

## Step 11: Run and Visualize with Peacock

```bash
cd ~/projects/moose/tutorials/darcy-thermo_mech/step11_boundary_conditions
make -j 12 # use number of processors for you system
cd problems
~/projects/moose/python/peacock/peacock -i heat_transient.i
```

!---

## Step 11: Results

!media step11_result.webm

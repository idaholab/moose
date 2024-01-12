# Step 3: Boundary conditions id=step03

!---

Fixed temperatures at the boundary conditions are simple but not realistic. We want to have:

- a fixed heat flux from the micro-reactor inside the concrete cavity

- natural convection boundary conditions with the air around the shield

- convective boundary conditions with the water block

!!end-intro

<!-- See index.md, the boundary conditions system is presented here -->

!---

First, the sidesets must be added / present in the mesh.
We modified the meshing script to generate sidesets for our boundary conditions

!listing step03_boundary_conditions/inputs/mesh.i

!---

Fixed heat flux


!listing framework/src/bcs/NeumannBC.C

!---

Fixed heat flux


!listing step03_boundary_conditions/inputs/step3.i block=BCs/from_reactor

!---

Natural convection with air


!listing step03_boundary_conditions/inputs/step3.i block=BCs/air_convection

!---

Convective boundary conditions

We use the same boundary condition as for with for now. When we introduce a separate variable for the
water temperature, we will revisit this.

!listing step03_boundary_conditions/inputs/step3.i block=BCs/water_convection

!---

## Step 3: Input File

!listing step03_boundary_conditions/inputs/step3.i

!---

## Step 3: Run

Using the step 3 executable:

```bash
cd ~/projects/moose/tutorials/shield_multiphysics/step03_boundary_conditions
make -j 12 # use number of processors for your system
cd inputs
../moose-opt -i step3.i
```

Using a prebuilt MOOSE from conda:

```
conda activate moose
cd ~/projects/moose/tutorials/shield_multiphysics/step03_boundary_conditions/inputs
moose-opt -i mesh.i --mesh-only
moose-opt -i step3.i
```

!---

## Step 3: Result

!media shield_multiphysics/results/step03.png style=width:70%;margin-left:auto;margin-right:auto

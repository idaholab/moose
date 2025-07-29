# Step 3: Boundary conditions id=step03

!---

Fixed temperatures at the boundary conditions are simple but not realistic. We want to have:

- a fixed heat flux from the micro-reactor inside the concrete cavity

- natural convection boundary conditions with the air around the shield

- convective boundary conditions with the water block

!!end-intro

!! See index.md, the boundary conditions system is presented there

!---

First, the sidesets must be added / present in the mesh.
We modified the meshing script to generate sidesets for our boundary conditions

- [RenameBoundaryGenerator](RenameBoundaryGenerator.md)
- [SideSetsBetweenSubdomainsGenerator](SideSetsBetweenSubdomainsGenerator.md)
- [SideSetsAroundSubdomainGenerator](SideSetsAroundSubdomainGenerator.md)

!row!

!col! width=50%

!style fontsize=40%
!listing step03_boundary_conditions/mesh.i

!col-end!

!col! width=50%

!media shield_multiphysics/results/step03_mesh.png style=width:100%;margin-left:auto;margin-right:auto;display:block

!col-end!

!row-end!

!---

We apply a [NeumannBC](NeumannBC.md) for fixed heat flux on the inner cavity.

!listing step03_boundary_conditions/step3.i block=BCs/from_reactor

!---

We apply a [DirichletBC](DirichletBC.md) for a fixed temperature with the `ground`.

!listing step03_boundary_conditions/step3.i block=BCs/ground

!---

Convection with air using [ConvectiveHeatFluxBC](ConvectiveHeatFluxBC.md).

!listing step03_boundary_conditions/step3.i block=BCs/air_convection

Note that sidesets are oriented. The variables on which the BC is applied must be defined on the
primary side (opposite the normal) of the sideset. For an external boundary, this is never a concern.

!---

Convection with water

We use the same boundary condition for the water boundary as for the convection with air for now. When we introduce a separate variable for the
water temperature, we will revisit this.
We use the 'water_boundary_inwards' surface because the solid block is on its primary side.

!listing step03_boundary_conditions/step3.i block=BCs/water_convection

!---

Summary of the boundary conditions:

!media shield_multiphysics/results/step03_bcs.png

!---

## Step 3: Input File

!listing step03_boundary_conditions/step3.i

!---

## Step 3: Run

```bash
cd ~/projects/moose/tutorials/shield_multiphysics/inputs/step03_boundary_conditions
moose-opt -i mesh.i --mesh-only
moose-opt -i step3.i
```

!---

## Step 3: Result

We only show the solid temperature. The water temperature is a constant for now.

!media shield_multiphysics/results/step03.png


# HeatStructure2DRadiationCouplerRZ

This component couples two cylindrical, [2D heat structures](component_groups/heat_structure_2d.md)
via opaque, gray, diffuse radiation.

## Usage

This component has the following restrictions:

- The coupled heat structures must derive from `HeatStructureCylindricalBase`,
  for example, [HeatStructureCylindrical.md].
- Only one boundary name may be provided in each of the
  [!param](/Components/HeatStructure2DRadiationCouplerRZ/primary_boundary) and
  [!param](/Components/HeatStructure2DRadiationCouplerRZ/secondary_boundary) parameters.
- The meshes along the coupled boundaries must be aligned. Each element on a
  boundary is paired with the nearest element on the coupled boundary. The
  alignment check requires that each element on a boundary has exactly one
  element from the coupled boundary paired to it.
- The boundaries must be radial, i.e., on either the inner or outer cylindrical
  surfaces, not the flat surfaces.

!syntax parameters /Components/HeatStructure2DRadiationCouplerRZ

## Formulation

!include heat_structure_formulation.md

!include heat_structure_boundary_formulation_neumann.md

This component computes and applies the boundary flux $q_b$ for each boundary.
For two opaque, gray, diffuse surfaces in an enclosure, the heat flux $q_i$ to
surface $i$ is the following [!citep](incropera2002):

!equation
q_i = \frac{\sigma (T_j^4 - T_i^4)}{\mathcal{R}_i} \eqc

where $\mathcal{R}_i$ is sometimes described as a radiation resistance:

!equation
\mathcal{R}_i = \frac{1 - \epsilon_i}{\epsilon_i} + \frac{1}{F_{i,j}}
  + \frac{1 - \epsilon_j}{\epsilon_j}\frac{A_i}{A_j} \eqc

where

- $\epsilon_i$ is the emissivity of surface $i$,
- $\sigma$ is the Stefan-Boltzmann constant,
- $F_{i,j}$ is the view factor from surface $i$ to surface $j$,
- $T_i$ is the temperature of surface $i$, and
- $A_i$ is the area of surface $i$.

The surface $i$ that is enclosed by the other surface has its view factor
set to unity:

!equation
F_{i,j} = 1 \eqc

whereas the other is computed using the reciprocity rule:

!equation
F_{j,i} = \frac{A_i}{A_j} \eqp

!syntax inputs /Components/HeatStructure2DRadiationCouplerRZ

!syntax children /Components/HeatStructure2DRadiationCouplerRZ

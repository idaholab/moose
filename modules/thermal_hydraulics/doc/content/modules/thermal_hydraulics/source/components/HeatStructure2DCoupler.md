# HeatStructure2DCoupler

This component couples two [2D heat structures](component_groups/heat_structure_2d.md)
via a heat transfer coefficient.

## Usage

This component has the following restrictions:

- The coupled heat structures must be [2D heat structures](component_groups/heat_structure_2d.md).
- The coupled heat structures must be of the same type.
- Only one boundary name may be provided in each of the
  [!param](/Components/HeatStructure2DCoupler/primary_boundary) and
  [!param](/Components/HeatStructure2DCoupler/secondary_boundary) parameters.
- The meshes along the coupled boundaries must be coincident, i.e., each node
  on each side must be at an identical location as a node on the other side.

!syntax parameters /Components/HeatStructure2DCoupler

## Formulation

!include heat_structure_formulation.md

!include heat_structure_boundary_formulation_neumann.md

For the heat structure $k$, the incoming boundary heat flux $q_b$ is computed as

\begin{equation}
  q_b = \mathcal{H} (T_j - T_k) \eqc
\end{equation}
where

- $\mathcal{H}$ is the heat transfer coefficient,
- $T_k$ is the surface temperature of the heat structure $k$, and
- $T_j$ is the surface temperature of the coupled heat structure $j$.

!syntax inputs /Components/HeatStructure2DCoupler

!syntax children /Components/HeatStructure2DCoupler

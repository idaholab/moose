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
- The meshes along the coupled boundaries must be aligned. Each element on a
  boundary is paired with the nearest element on the coupled boundary. The
  alignment check requires that each element on a boundary has exactly one
  element from the coupled boundary paired to it.

!syntax parameters /Components/HeatStructure2DCoupler

## Formulation

!include heat_structure_formulation.md

!include heat_structure_boundary_formulation_neumann.md

For the heat structure $k$, the incoming boundary heat flux $q_b$ is computed as

\begin{equation}
  q_b = \mathcal{H} (T_j - T_k) F_k \eqc
\end{equation}
where

- $\mathcal{H}$ is the heat transfer coefficient,
- $T_k$ is the surface temperature of the heat structure $k$,
- $T_j$ is the surface temperature of the coupled heat structure $j$, and
- $F_k$ is the area scaling factor of the heat structure $k$:

!equation
F_k = \left\{\begin{array}{l l}
  1 & A_k \leq A_j\\
  \frac{A_j}{A_k} & A_k > A_j\\
  \end{array}\right. \eqc

where $A_k$ is the area of the heat structure boundary $k$.

!syntax inputs /Components/HeatStructure2DCoupler

!syntax children /Components/HeatStructure2DCoupler

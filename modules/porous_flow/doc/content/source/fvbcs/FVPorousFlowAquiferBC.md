# FVPorousFlowAquiferBC

!syntax description /FVBCs/FVPorousFlowAquiferBC

`FVPorousFlowAquiferBC` is the finite-volume analogue of
[`PorousFlowAquiferBC`](PorousFlowAquiferBC.md).  It applies a Robin (Cauchy) boundary
condition coupling the model boundary to a far-field aquifer:
\begin{equation}
  s = \chi \, k_{\mathrm{r}} \, C \left( P_{\mathrm{cell}} - P_{\mathrm{aq}}(z) \right) ,
\end{equation}
where $C$ is the conductance (kg.m$^{-2}$.Pa$^{-1}$.s$^{-1}$), $P_{\mathrm{cell}}$ is the
pore pressure of the boundary cell, $\chi$ is the mass fraction of the fluid component in
the phase, and $k_{\mathrm{r}}$ is the phase relative permeability.  A positive $s$ means
fluid leaves the domain.

The elevation $z$ is that of the +boundary-cell centroid+ (not the face centroid), computed
from the user-supplied gravity vector as $z = -\mathbf{x}_{\mathrm{cell}} \cdot
\hat{\mathbf{g}}$.  This is deliberate: the FV pore pressure is a cell-centred quantity, so
evaluating the aquifer pressure at the cell-centroid elevation is what makes the flux exactly
zero in hydrostatic equilibrium, on a boundary of any orientation.

Using the face centroid instead would offset $P_{\mathrm{aq}}$ from $P_{\mathrm{cell}}$ by
$\rho |\mathbf{g}| (z_{\mathrm{face}} - z_{\mathrm{cell}})$ and so drive a spurious flux
through an equilibrated boundary.  That offset vanishes only when the two centroids sit at
the same elevation.  For axis-aligned cells that is the case on a vertical boundary (face
normal perpendicular to gravity), but not otherwise: on a horizontal boundary, such as the
top or bottom of the domain, the face centroid sits half a cell height above or below the
cell centroid, and on slanted or distorted cells the offset takes some intermediate value.
All the other quantities entering the flux ($\rho$, $\mu$, $\chi$, $k_{\mathrm{r}}$ and the
permeability) are likewise boundary-cell values, so the boundary condition is evaluated
consistently at the cell centroid.

The two reference-pressure formulations and the conductance treatment are identical to
[`PorousFlowAquiferBC`](PorousFlowAquiferBC.md): exactly one of `aquifer_head` (with an
explicit `aquifer_conductance`) or `aquifer_pressure_at_datum` (with `aquifer_distance`, and
optionally `aquifer_permeability` to override the boundary-cell permeability) must be
supplied.

Two differences from the FE object:

- Following the convention of the other FV PorousFlow objects, the flux is always multiplied
  by the mass fraction $\chi$ and relative permeability $k_{\mathrm{r}}$ of the boundary
  cell (in the FE object these are optional flags inherited from
  [`PorousFlowSink`](PorousFlowSink.md)).  For a single-phase, single-component model with
  $k_{\mathrm{r}} = 1$ the two objects apply the same flux.
- Derivatives are computed by automatic differentiation.

## Comparison with `FVPorousFlowAdvectiveFluxBC`

[`FVPorousFlowAdvectiveFluxBC`](FVPorousFlowAdvectiveFluxBC.md) imposes a fixed scalar
pressure at the boundary face via a flux-consistent Darcy expression; its effective
conductance is the boundary-cell mobility divided by the cell-to-face distance, which is a
mesh-dependent quantity that tends to a Dirichlet condition on refinement.
`FVPorousFlowAquiferBC` instead has a physical, mesh-independent conductance (spanning the
full Dirichlet-to-Neumann range via the aquifer distance $L$) and an elevation-corrected
reference pressure, so it remains correct on boundaries with vertical extent.

See [boundary conditions](boundaries.md) and [`PorousFlowAquiferBC`](PorousFlowAquiferBC.md)
for the formulation details and the effect of the aquifer distance.

!syntax parameters /FVBCs/FVPorousFlowAquiferBC

!syntax inputs /FVBCs/FVPorousFlowAquiferBC

!syntax children /FVBCs/FVPorousFlowAquiferBC

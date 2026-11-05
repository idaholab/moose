# MortarContactForce

!syntax description /Postprocessors/MortarContactForce

## Overview

`MortarContactForce` reports the total normal contact force that one mortar contact pair transmits
to its secondary surface. It reads the pair's weighted gap user object, so it works for every pair of
a [Contact](syntax/Contact/index.md) block with several pairs, without a separate residual tag for
each pair.

With the nodal contact pressures $\lambda_j$, the nodal normals $\mathbf{n}_j$ and the Lagrange
multiplier basis $\Phi_j$ of the secondary surface $\gamma_c^{(1)}$, the contact traction is
$\sum_j \Phi_j \lambda_j \mathbf{n}_j$, and the force is

\begin{equation}
\mathbf{F} = \sum_j \lambda_j \mathbf{n}_j \int_{\gamma_c^{(1)}} \Phi_j \, dA,
\end{equation}

where the integrals are taken over the mortar segments. The weighted gap user object holds these
integrals and the physical pressures $\lambda_j$, also under node-based scaling of the Lagrange
multipliers.

The [!param](/Postprocessors/MortarContactForce/weighted_gap_uo) created by the Contact action is
named `lm_weightedgap_object_<block name>`, followed by `_p_<primary>_s_<secondary>` when the block
has several pairs.

## Example Input File Syntax

!listing test/tests/mortar_contact_force/mortar_contact_force.i block=Postprocessors

!syntax parameters /Postprocessors/MortarContactForce

!syntax inputs /Postprocessors/MortarContactForce

!syntax children /Postprocessors/MortarContactForce

# MortarContactForce

## Overview

`MortarContactForce` reports the normal contact force that one mortar contact pair transmits to its
secondary surface. It reads the pair's weighted gap user object, so each pair of a
[Contact](syntax/Contact/index.md) block with several pairs can be reported without a separate
residual tag per pair.

With the nodal contact pressures $\lambda_j$ and nodal normals $\mathbf{n}_j$ of the secondary
surface $\gamma_c^{(1)}$, the force is

\begin{equation}
\mathbf{F} = \sum_j \lambda_j \mathbf{n}_j \int_{\gamma_c^{(1)}} N_j \, dA,
\end{equation}

where $N_j$ are the standard Lagrange shape functions and the integrals are taken over the mortar
segments. This is the integral of the contact traction for the standard, dual and Petrov-Galerkin
bases, since a dual basis function $\Phi_j$ has the same integral as $N_j$. The pressures
$\lambda_j$ are the physical pressures, also under node-based scaling of the Lagrange multipliers.

The force is normal only; with friction, `magnitude` does not include the tangential force.

The [!param](/Postprocessors/MortarContactForce/weighted_gap_uo) created by the Contact action is
named `lm_weightedgap_object_<block name>` for the `mortar` formulation and
`penalty_weightedgap_object_<block name>` for `mortar_penalty`, followed by
`_p_<primary>_s_<secondary>` when the block has several pairs.

## Example Input File Syntax

!listing test/tests/mortar_contact_force/mortar_contact_force.i block=Postprocessors

!syntax parameters /Postprocessors/MortarContactForce

!syntax inputs /Postprocessors/MortarContactForce

!syntax children /Postprocessors/MortarContactForce

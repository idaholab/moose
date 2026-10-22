# KokkosMortarContactKinematicsUserObject

!if! function=hasCapability('kokkos')

!syntax description /UserObjects/KokkosMortarContactKinematicsUserObject

This is the Kokkos counterpart of [LMWeightedGapUserObject](LMWeightedGapUserObject.md). For each
secondary node $j$ of a mortar interface on the current configuration it integrates the weighted gap

!equation
\tilde{g}_j = \int_{\Gamma_c} \psi_j \, (\boldsymbol{x}^{(2)} - \boldsymbol{x}^{(1)}) \cdot \boldsymbol{n}_j \, d\Gamma

and its normalization $\int_{\Gamma_c} \psi_j \, d\Gamma$ over the mortar segments, where $\psi_j$ is
the basis function of the Lagrange multiplier at node $j$ (its dual basis function when
[!param](/UserObjects/KokkosMortarContactKinematicsUserObject/lm_variable) uses one) and $\boldsymbol{n}_j$ is
the nodal normal. The owner of a node sums the contributions of all processes, and every process
whose local secondary elements touch the node receives the sum. The object computes geometry only:
[KokkosComputeWeightedGapLMMechanicalContact](KokkosComputeWeightedGapLMMechanicalContact.md)
decides the contact conditions from it.

With [!param](/UserObjects/KokkosMortarContactKinematicsUserObject/compute_slip) set to true, it also integrates
the weighted gap vector $\int_{\Gamma_c} \psi_j (\boldsymbol{x}^{(2)} - \boldsymbol{x}^{(1)}) \,
d\Gamma$ and the weighted relative displacement increment over the time step, which frictional
contact reads.

The Lagrange multiplier must be a first-order Lagrange variable. The object always uses the correct
edge dropping treatment.

## Example Input Syntax

!listing contact/test/tests/kokkos/mortar/frictionless_weighted_gap.i block=UserObjects

!syntax parameters /UserObjects/KokkosMortarContactKinematicsUserObject

!syntax inputs /UserObjects/KokkosMortarContactKinematicsUserObject

!syntax children /UserObjects/KokkosMortarContactKinematicsUserObject

!if-end!

!else
!include kokkos/kokkos_warning.md

# ComputeDSGShellResultants

!syntax description /Materials/ComputeDSGShellResultants

## Description

This material computes the stress resultants of a DSG shell element as the product of the section stiffness of [ComputeDSGShellSectionStiffness](/ComputeDSGShellSectionStiffness.md) with the generalized strains of [ComputeDSGShellStrain](/ComputeDSGShellStrain.md). The resultants are per unit length and in the element frame, in the order

$N_{11}, N_{22}, N_{12}, M_{11}, M_{22}, M_{12}, Q_1, Q_2, M_d$

that is, the membrane forces, the bending moments (positive when the face the normal points to is in tension, the convention of the module's four-node shell), the transverse shear forces and the drilling moment. They are stored in the material property `dsg_shell_resultants`, read by [StressDivergenceDSGShell](/StressDivergenceDSGShell.md) and written to auxiliary variables by [DSGShellResultantsAux](/DSGShellResultantsAux.md). See [DSGShellElements](/DSGShellElements.md) for the sign conventions.

When the strain material was asked for the strains at the nodes (`compute_nodal_strains`), this material also computes the resultants at the nodes, `dsg_shell_nodal_resultants` (one row per node), read by [DSGShellNodalResultants](/DSGShellNodalResultants.md).

A different section law, for example a layered or an inelastic section, replaces this material and keeps the strain material and the kernel.

## Example Input File syntax

!listing modules/solid_mechanics/test/tests/shell_dsg/static/cantilever_strip.i block=Materials

!syntax parameters /Materials/ComputeDSGShellResultants

!syntax inputs /Materials/ComputeDSGShellResultants

!syntax children /Materials/ComputeDSGShellResultants

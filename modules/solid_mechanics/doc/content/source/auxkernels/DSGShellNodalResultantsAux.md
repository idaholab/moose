# DSGShellNodalResultantsAux

!syntax description /AuxKernels/DSGShellNodalResultantsAux

## Description

This auxiliary kernel writes one stress resultant of the DSG shell elements, averaged at the nodes by a [DSGShellNodalResultants](/DSGShellNodalResultants.md) user object, to a nodal (Lagrange) auxiliary variable. The resultants are per unit length and in the element frame, with the names and the sign conventions of [DSGShellResultantsAux](/DSGShellResultantsAux.md); the elemental values of that kernel and the nodal values of this one come from the same strains and section stiffness.

## Example Input File syntax

!listing modules/solid_mechanics/test/tests/shell_dsg/static/nodal_resultants.i block=AuxVariables/moment_22_nodal AuxKernels/moment_22_nodal

!syntax parameters /AuxKernels/DSGShellNodalResultantsAux

!syntax inputs /AuxKernels/DSGShellNodalResultantsAux

!syntax children /AuxKernels/DSGShellNodalResultantsAux

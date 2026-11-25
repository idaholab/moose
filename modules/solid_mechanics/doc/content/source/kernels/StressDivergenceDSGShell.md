# StressDivergenceDSGShell

!syntax description /Kernels/StressDivergenceDSGShell

## Description

This kernel computes the internal force of a DSG shell element on one of its six degrees of freedom and the exact Jacobian of that force. The residual is the product of the strain-displacement matrix of [ComputeDSGShellStrain](/ComputeDSGShellStrain.md) with the stress resultants of [ComputeDSGShellResultants](/ComputeDSGShellResultants.md), and the Jacobian the product of the strain-displacement matrix with the section stiffness of [ComputeDSGShellSectionStiffness](/ComputeDSGShellSectionStiffness.md) and the strain-displacement matrix. Both are written out because the element is linear in its degrees of freedom; see [DSGShellElements](/DSGShellElements.md).

One kernel is needed for each of the six degrees of freedom. The `component` parameter selects it: 0, 1 and 2 for the displacements along the global axes and 3, 4 and 5 for the rotations about them. The variable of the kernel must be the one listed for that component in `displacements` and `rotations`.

## Example Input File syntax

!listing modules/solid_mechanics/test/tests/shell_dsg/static/cantilever_strip.i block=Kernels

!syntax parameters /Kernels/StressDivergenceDSGShell

!syntax inputs /Kernels/StressDivergenceDSGShell

!syntax children /Kernels/StressDivergenceDSGShell

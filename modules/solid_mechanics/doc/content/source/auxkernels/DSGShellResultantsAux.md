# DSGShellResultantsAux

!syntax description /AuxKernels/DSGShellResultantsAux

## Description

This auxiliary kernel writes one stress resultant of a DSG shell element, computed by [ComputeDSGShellResultants](/ComputeDSGShellResultants.md), to an elemental auxiliary variable. The resultants are per unit length and in the element frame: the membrane forces $N_{11}$, $N_{22}$ and $N_{12}$, the bending moments $M_{11}$, $M_{22}$ and $M_{12}$ ($M = \int \sigma z \, \mathrm{d}z$, positive when the face the normal $\mathbf{e}_3$ points to is in tension, as in [ShellResultantsAux](/ShellResultantsAux.md)), the transverse shear forces $Q_1$ and $Q_2$, and the drilling moment. The frame and the sign conventions are described in [DSGShellElements](/DSGShellElements.md). On a `CONSTANT MONOMIAL` variable the value is the average of the resultant over the quadrature points of the element.

### Choosing the parameters

- `resultant` picks one of the nine resultants, per unit length and in the element frame: the membrane forces $N_{11}$, $N_{22}$, $N_{12}$; the bending moments $M_{11}$, $M_{22}$, $M_{12}$, positive when the face the normal points to is in tension; the transverse shear forces $Q_1$, $Q_2$; the drilling moment. Direction 1 is the first local axis set by `reference_first_local_direction` of the strain material.
- `variable` must be an elemental variable, `CONSTANT MONOMIAL`; it receives the average over the quadrature points of the element.

## Example Input File syntax

!listing modules/solid_mechanics/test/tests/shell_dsg/static/cantilever_strip.i block=AuxKernels

!syntax parameters /AuxKernels/DSGShellResultantsAux

!syntax inputs /AuxKernels/DSGShellResultantsAux

!syntax children /AuxKernels/DSGShellResultantsAux

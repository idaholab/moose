# DiscreteNucleationForce

!syntax description /Kernels/DiscreteNucleationForce

It implements the weak form term

\begin{equation}
(-M(r) \cdot (v_1 - v_0) + v_0, \psi),
\end{equation}

where $u$ ist the variable the kernel is acting on, $M$ is the
[DiscreteNucleationMap](/DiscreteNucleationMap.md) data, and $\psi$ is a  test
function. The map values $M(r)$ range from 0..1 and are remapped using the $v_0$
(`no_nucleus_value`) and $v_1$ (`nucleus_value`) parameters.

This kernel can be used together with the [Reaction](/Reaction.md) kernel to set
a non-linear variable field to an affine transformation of the discrete
nucleation map data.

The `DiscreteNucleationForce` kernel is part of the
[Discrete Nucleation system](Nucleation/DiscreteNucleation.md). Its companion
AuxKernel is [DiscreteNucleationAux](/DiscreteNucleationAux.md).

!syntax parameters /Kernels/DiscreteNucleationForce

!syntax inputs /Kernels/DiscreteNucleationForce

!syntax children /Kernels/DiscreteNucleationForce

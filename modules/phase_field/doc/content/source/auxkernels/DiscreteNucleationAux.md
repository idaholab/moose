# DiscreteNucleationAux

!syntax description /AuxKernels/DiscreteNucleationAux

This AuxKernel computes the value $v$ as

\begin{equation}
v = M(r) \cdot (v_1 - v_0) + v_0
\end{equation}

where $v$ ist the variable the auxkernel is acting on and $M$ is the
[DiscreteNucleationMap](/DiscreteNucleationMap.md) data. The map values $M(r)$
range from 0..1 and are remapped using the $v_0$ (`no_nucleus_value`) and $v_1$
(`nucleus_value`) parameters.

The `DiscreteNucleationAux` auxkernel is part of the
[Discrete Nucleation system](Nucleation/DiscreteNucleation.md). Its companion
kernel is [DiscreteNucleationForce](/DiscreteNucleationForce.md).

!syntax parameters /AuxKernels/DiscreteNucleationAux

!syntax inputs /AuxKernels/DiscreteNucleationAux

!syntax children /AuxKernels/DiscreteNucleationAux

!bibtex bibliography

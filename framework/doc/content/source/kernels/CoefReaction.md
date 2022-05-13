# CoefReaction / ADCoefReaction

!syntax description /Kernels/CoefReaction

[`Reaction`](/Reaction.md) with a scalar prefactor $\lambda$ (`coef`)
\begin{equation}
(\lambda v, \psi),
\end{equation}
where $v$ (`v`) is a coupled variable.

!alert note
There is no FV (finite volume) version of `CoefReaction`. If you wish to use FV,
use [/FVCoupledForce.md].

!syntax parameters /Kernels/CoefReaction

!syntax inputs /Kernels/CoefReaction

!syntax children /Kernels/CoefReaction

!bibtex bibliography

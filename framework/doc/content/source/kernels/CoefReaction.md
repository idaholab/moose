# CoefReaction

!syntax description /Kernels/CoefReaction

[`Reaction`](/Reaction.md) with a scalar prefactor $\lambda$ (`coef`)
\begin{equation}
(\lambda v, \psi),
\end{equation}
where $v$ (`v`) is a coupled variable.

!alert note
There is no AD (automatic differentiation) or FV (finite volume) version of `CoefReaction`.
If you wish to use AD / FV, use [`ADCoupledForce`](/ADCoupledForce.md) /
[`FVCoupledForce`](/FVCoupledForce.md) respectively.

!syntax parameters /Kernels/CoefReaction

!syntax inputs /Kernels/CoefReaction

!syntax children /Kernels/CoefReaction

!bibtex bibliography

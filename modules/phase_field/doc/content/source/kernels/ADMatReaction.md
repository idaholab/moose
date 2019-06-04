# ADMatReaction

!syntax description /Kernels/ADMatReaction

Implements
\begin{equation}
(-L(v,a,b,\dots) v, \psi),
\end{equation}
where $L$ (`mob_name`) is a reaction rate, $v$ is either a coupled variable
(`v`) or - if not explicitly specified - the non-linear variable the kernel is
operating on.

Forward automatic differentiation is used to compute all on and off-diagonal
Jacobian contributions.

!alert warning
Note the negative sign, which does *not* appear in [Reaction](/Reaction.md) or
[CoefReaction](/CoefReaction.md).

!syntax parameters /Kernels/ADMatReaction

!syntax inputs /Kernels/ADMatReaction

!syntax children /Kernels/ADMatReaction

# ADMatReaction

!syntax description /Kernels/ADMatReaction

Implements
\begin{equation}
(-L(v,a,b,\dots) v, \psi),
\end{equation}
where $L$ ([!param](/Kernels/ADMatReaction/reaction_rate)) is a reaction rate, $v$ is either
a coupled variable ([!param](/Kernels/ADMatReaction/v)) or - if not explicitly specified - 
the non-linear variable the kernel is operating on.

Forward automatic differentiation is used to compute all on and off-diagonal
Jacobian contributions.

!alert warning
Note the negative sign, which does *not* appear in [Reaction](/Reaction.md) or
[CoefReaction](/CoefReaction.md).

## Example Input File Syntax

!listing ad_mat_reaction.i block=Kernels/reaction

!syntax parameters /Kernels/ADMatReaction

!syntax inputs /Kernels/ADMatReaction

!syntax children /Kernels/ADMatReaction

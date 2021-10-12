# MatReaction

!syntax description /Kernels/MatReaction

Implements
\begin{equation}
(-L(v,a,b,\dots) v, \psi),
\end{equation}
where $L$ (`mob_name`) is a reaction rate, $v$ is either a coupled variable (`v`)
or - if not explicitly specified - the nonlinear variable the kernel is operating on.

!alert warning
Note the negative sign, which does *not* appear in [Reaction](/Reaction.md) or
[CoefReaction](/CoefReaction.md).

!syntax parameters /Kernels/MatReaction

!syntax inputs /Kernels/MatReaction

!syntax children /Kernels/MatReaction

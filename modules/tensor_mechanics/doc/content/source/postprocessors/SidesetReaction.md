# Side Reaction Postprocessor

!syntax description /Postprocessors/SidesetReaction

## Description

`SidesetReaction` computes the integral of the force $F$ along vector $\vec{e}$ acting on a sideset $\partial S$:

\begin{equation}
  F = \int_{\partial S} \vec{n}^T ~ \underline{\sigma}  \vec{e}  dS,
\end{equation}

where $\underline{\sigma}$ is the stress tensor.

!syntax parameters /Postprocessors/SidesetReaction

!syntax inputs /Postprocessors/SidesetReaction

!syntax children /Postprocessors/SidesetReaction

# StickyBC

!syntax description /BCs/StickyBC


## Description

This nodal boundary condition imposes the condition
\begin{equation}
u = u_{\mathrm{old}} \ ,
\end{equation}
if $u_{\mathrm{old}}$ exceeds either
\begin{equation}
u_{\mathrm{old}} \geq u_{\mathrm{max}}
\end{equation}
or
\begin{equation}
u_{\mathrm{old}} \leq u_{\mathrm{min}} \ .
\end{equation}
Hence the name "sticky": as soon as $u$ exceeds the bounds it is fixed at subsequent timesteps.

!alert note
`StickyBC` should be used with care.  It only approximates a Constraint imposed by a wall, for
instance, for: the boundary nodes may penetrate the wall in a single timestep before they are then
stuck (inside the wall); boundary nodes can never move away from the wall again once they are stuck.
However, it offers the advantage over a full-blown Constraint that it does not adversely affect
numerical convergence.

!syntax parameters /BCs/StickyBC

!syntax inputs /BCs/StickyBC

!syntax children /BCs/StickyBC

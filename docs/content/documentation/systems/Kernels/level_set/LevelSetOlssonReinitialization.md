# LevelSetOlssonReinitialization

This kernel implements the non-time portion of the  conservative "re-initialization" algorithm of \citet{olsson2007conservative} that
transforms $u_h$ into a smooth function in the range $[0, 1]$ rather than a signed distance, which is
useful for certain types of problems such as phase identification. The weak form of the re-initialization equation is:
find $U_h$ such that


\begin{equation}
  \left(\frac{\partial U_h}{\partial \tau}, \psi_i\right) + \underbrace{\left(\nabla\psi_i, (-f + \epsilon\nabla U_h\cdot\hat{n}_{\ast})\hat{n}_{\ast}\right)}_{\textrm{LevelSetOlssonReinitialization}} = 0,
\end{equation}

where $\tau$ is the pseudo time
during the re-initialization, $\hat{n}_{\ast}$ is the normal vector
computed from the level set variable $u_h$ at pseudo time $\tau=0$,
$\epsilon$ is the interface thickness, $f\equiv U_h(1-U_h)$, and
$U_h(\tau=0) = u_h$.

When steady-state for $U_h$ is detected when (see [LevelSetOlssonTerminator](level_set/LevelSetOlssonTerminator.md))
the entire process is repeated at time $t+\Delta t$.

## Example Syntax
The LevelSetOlssonReinitialization kernel is typically employed in a reinitialization sub-app (see the MOOSE
[MultiApp](/MultiApps/index.md) system), within this context this Kernel is invoked with the `[Kernels]` as follows.

!listing modules/level_set/tests/reinitialization/reinit.i block=Kernels label=False

!parameters /Kernels/LevelSetOlssonReinitialization

!inputfiles /Kernels/LevelSetOlssonReinitialization

!childobjects /Kernels/LevelSetOlssonReinitialization

## References

\bibliographystyle{unsrt}
\bibliography{docs/bib/level_set.bib}

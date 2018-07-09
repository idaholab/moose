# LevelSetOlssonTerminator

This object is utilized to terminate a level set reinitialization solve, when steady-state for $U_h$
(see [/LevelSetOlssonReinitialization.md]) is detected:

\begin{equation}
  \label{eqn:steady_state}
  \frac{\|U_h^{m+1} - U_h^{m}\|}{\Delta\tau} < \delta,
\end{equation}
where $\delta$ is a problem-dependent tolerance, and $U_h^m \equiv U_h(\tau=m\Delta \tau)$.  When
steady-state is achieved, $u_h$ is set equal to the re-initialized solution $U_h$, and the entire
process is repeated at time $t+\Delta t$.

## Example Syntax

!listing modules/level_set/test/tests/reinitialization/reinit.i block=UserObjects

!syntax parameters /UserObjects/LevelSetOlssonTerminator

!syntax inputs /UserObjects/LevelSetOlssonTerminator

!syntax children /UserObjects/LevelSetOlssonTerminator

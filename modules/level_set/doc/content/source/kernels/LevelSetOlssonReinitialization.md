# LevelSetOlssonReinitialization

This kernel implements the non-time portion of the conservative "re-initialization" algorithm
of [!cite](olsson2007conservative) that transforms $u_h$ into a smooth function in the range $[0, 1]$
rather than a signed distance, which is useful for certain types of problems such as phase
identification. The weak form of the original re-initialization equation is: find $U_h$ such that

\begin{equation}
  \left(\frac{\partial U_h}{\partial \tau}, \psi_i\right) + \underbrace{\left(\nabla\psi_i, -f\hat{n}_{\ast} + \epsilon\nabla U_h\right)}_{\textrm{LevelSetOlssonReinitialization}} = 0
\end{equation}
where $\tau$ is the pseudo time
during the re-initialization, $\hat{n}_{\ast}$ is the normal vector
computed from the level set variable $u_h$ at pseudo time $\tau=0$,
$\epsilon$ is the interface thickness, $f\equiv U_h(1-U_h)$, and
$U_h(\tau=0) = u_h$.

To avoid any tangential diffusion, a modified re-initialization formulation is proposed by restricting compressive flux to normal direction. The weak form of the modified re-initialization equation is: find $U_h$ such that

\begin{equation}
  \left(\frac{\partial U_h}{\partial \tau}, \psi_i\right) + \underbrace{\left(\nabla\psi_i, -f\hat{n}_{\ast} + \epsilon\nabla U_h\cdot\hat{n}_{\ast}\hat{n}_{\ast}\right)}_{\textrm{LevelSetOlssonReinitialization}} = 0,
\end{equation}
where $\tau$ is the pseudo time
during the re-initialization, $\hat{n}_{\ast}$ is the normal vector
computed from the level set variable $u_h$ at pseudo time $\tau=0$,
$\epsilon$ is the interface thickness, $f\equiv U_h(1-U_h)$, and
$U_h(\tau=0) = u_h$.

When steady-state for $U_h$ is detected when (see [/LevelSetOlssonTerminator.md] the entire process
is repeated at time $t+\Delta t$.

Our preliminary numerical tests indicate that the original re-initialization formulation works better to maintain the smoothness of an interface, so the original re-initialization formulation is set to be default. The modified re-initialization formulation can be used by setting `use_modified_reinitilization_step = true` although additional work needs to be done to investigate its numerical instability issue.

## Example Syntax

The LevelSetOlssonReinitialization kernel is typically employed in a reinitialization sub-app (see
the MOOSE [MultiApp](/MultiApps/index.md) system), within this context this Kernel is invoked with
the `[Kernels]` as follows.

!listing modules/level_set/test/tests/reinitialization/reinit.i block=Kernels

!syntax parameters /Kernels/LevelSetOlssonReinitialization

!syntax inputs /Kernels/LevelSetOlssonReinitialization

!syntax children /Kernels/LevelSetOlssonReinitialization



!bibtex bibliography

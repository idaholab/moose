# CFLTimeStepSize

!syntax description /Postprocessors/CFLTimeStepSize

For explicit time integration schemes, the CFL condition requires that the time
step size $\Delta t$ not be greater than the maximum distance that the fastest wave can
travel:
\begin{equation}
  \frac{\lambda_{max} \Delta t}{\Delta x_{min}} = C < C_{limit} \,,
\end{equation}
where $\lambda_{max}$ is the fastest wave speed in the domain,
$\Delta x_{min}$ is the smallest element size in the domain,
$C$ is the CFL number, and
$C_{limit}$ is the limit of the CFL number for the time integrator, typically
equal to 1.
For this post-processor, the user imposes a CFL number $C_{user}$, so the
time step size is computed as follows:
\begin{equation}
  \Delta t = C_{user}\frac{\Delta x_{min}}{\lambda_{max}} \,.
\end{equation}

This post-processor is valid for both 1-phase and 2-phase flow.
The maximum wave speed $\lambda_{max}$ is the maximum wave speed of any phase $k$:
\begin{equation}
  \lambda_{max} = \max\limits_k \lambda_{k,max} \,,
\end{equation}
where the maximum wave speed for a phase is approximated from the eigenvalues
in a conservative fashion:
\begin{equation}
  \lambda_{k,max} = |u_k| + c_k \,,
\end{equation}
where $u_k$ is the velocity, and $c_k$ is the sound speed.

!syntax parameters /Postprocessors/CFLTimeStepSize

!syntax inputs /Postprocessors/CFLTimeStepSize

!syntax children /Postprocessors/CFLTimeStepSize

!bibtex bibliography

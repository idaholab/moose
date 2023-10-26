# PseudoTimestep

The methods implemented in this postprocessor compute timesteps adaptively using Pseudo Transient Continuation strategies. This allows for steady state problems solved via time marching approcah to reach the final state rapidly.
As a reminder the time marching approach to computing steady states reframes a problem in discrete form $M\frac{\partial \mathbf u}{\Delta t}=F(\mathbf u)$ as

\begin{equation}
M\frac{\mathbf u_{n+1}-\mathbf u_n}{\Delta t}=F(\mathbf u_{n+1})
\end{equation}

## Overview

This object computes a timestep to accelerate the convergence to steay state using a pseudo transient.
The change in timestep is determined by the residual behaviour from one iteration to another, i.e. small changes in  residual indicate larger timesteps are allowed, while large changes indicate a decrease is necessary.
There main methods are recognized and implemented.


Methods supported


- Switched evolution relaxation (SER)

\begin{equation}
\Delta t_k =\Delta t_{k-1} \cdot \bigg(\frac{R_{k-\ell}}{R_{k}}\bigg)^{\alpha}
\end{equation}

where $\alpha$ is a user chosen parameter. The residual at step $k$ is $R_k$, and the residual at $\ell$ iterations before is denoted as $R_{k-\ell}$.

- Exponential progression (EXP)

\begin{equation}
\Delta t_k =\Delta t_{0} \cdot \alpha^k
\end{equation}

where $\alpha$ is a user chosen parameter, $k$ is the surrent iteration step.

- Residual Difference Method (RDM)

\begin{equation}
\Delta t_k =\Delta t_{k-1} \cdot \alpha^{\frac{R_{k-1}-R_k}{R_{k-1}}}
\end{equation}

!syntax parameters /Postprocessors/PseudoTimestep

!syntax inputs /Postprocessors/PseudoTimestep

!syntax children /Postprocessors/PseudoTimestep

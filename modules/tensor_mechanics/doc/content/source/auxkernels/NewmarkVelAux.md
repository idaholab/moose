# NewmarkVelAux

!syntax description /AuxKernels/NewmarkVelAux

## Description

This class computes the current velocity ($\mathbf{\dot{u}}(t+\Delta t)$) given the old velocity ($\mathbf{u}(t)$), old acceleration ($\mathbf{\ddot{u}}(t)$) and current acceleration ($\mathbf{\ddot{u}}(t+\Delta t)$) as follows:

\begin{equation}
\mathbf{\dot{u}}(t+ \Delta t) = \mathbf{\dot{u}}(t)+ (1-\gamma)\Delta t \mathbf{\ddot{u}}(t) + \gamma \Delta t \mathbf{\ddot{u}}(t+\Delta t)
\end{equation}

Here, $\gamma$ is the Newmark time integration parameter and $\Delta t$ is the time step. More information about the Newmark method can be found at [Dynamics](Dynamics.md).

!syntax parameters /AuxKernels/NewmarkVelAux

!syntax inputs /AuxKernels/NewmarkVelAux

!syntax children /AuxKernels/NewmarkVelAux

# NewmarkAccelAux

!syntax description /AuxKernels/NewmarkAccelAux

## Description

This class computes the current acceleration ($\mathbf{\ddot{u}}(t+\Delta t)$) given the current displacement ($\mathbf{u}(t+\Delta t)$), old displacement ($\mathbf{u}(t)$), old velocity ($\mathbf{\dot{u}}(t)$) and old acceleration ($\mathbf{\ddot{u}}(t)$) as follows:

\begin{equation}
\mathbf{\ddot{u}}(t+\Delta t) = \frac{\mathbf{u}(t+\Delta t)-\mathbf{u}(t)}{\beta \Delta t^2}- \frac{\mathbf{\dot{u}}(t)}{\beta \Delta t}+\frac{\beta -0.5}{\beta}\mathbf{\ddot{u}}(t)
\end{equation}

Here, $\beta$ is the Newmark time integration parameter and $\Delta t$ is the time step. More information about the Newmark method can be found at [Dynamics](Dynamics.md).

!syntax parameters /AuxKernels/NewmarkAccelAux

!syntax inputs /AuxKernels/NewmarkAccelAux

!syntax children /AuxKernels/NewmarkAccelAux

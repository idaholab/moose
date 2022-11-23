# StiffenedGasFluidProperties

!syntax description /FluidProperties/StiffenedGasFluidProperties

A simple fluid class that implements a stiffened equation of state [!citep](metayer2004)
\begin{equation}
  p = (\gamma - 1) \rho (e - q) - \gamma p_{\infty},
\end{equation}
where $\gamma = c_p/c_v$ is the ratio of specific heat capacities, $q$ is a constant that defines the
zero reference state for internal energy, and $p_{\infty}$ is a constant representing the attraction
between fluid molecules that makes the fluid *stiff* in comparison to an ideal gas. This equation of
state is typically used to represent water that is under very high pressure.

!syntax parameters /FluidProperties/StiffenedGasFluidProperties

!syntax inputs /FluidProperties/StiffenedGasFluidProperties

!syntax children /FluidProperties/StiffenedGasFluidProperties

!bibtex bibliography

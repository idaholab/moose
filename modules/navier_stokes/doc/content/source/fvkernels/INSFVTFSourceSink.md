# INSFVTFSourceSink

The object computes the turbulent source and sink term for the elliptic function.
This function models the pressure-driven relaxation of turbulence near the wall.

The net production minus destruction for the wall-normal stresses reads as follows:

\begin{equation}
- \frac{1}{L^2} \left( f + \frac{1}{k} (\delta_{v2} - C_2 G_k) \right) \,,
\end{equation}

where:

- $L^2$ is the square of the turbulent length scale,
- $k$ is the turbulent dynamic viscosity,
- $\delta_{v2}$ is the diffusivity function for the wall normal stress,
- $C_2 = 0.3$ is a model parameter,
- $G_k = \mu_t S^2$ is the production of turbulent kinetic energy with $\mu_t$ being the dynamic turbulent viscosity and $S$ is the shear strain tensor internal norm.

The diffusivity function for wall normal stresses is modeled as follows:

\begin{equation}
\frac{(C_1 - n) \overline{v^2} - \frac{2}{3} k (C_1 - 1.0)}{T_s} \,,
\end{equation}

where:

- $C_1 = 1.4$ and $n = 6.0$ are model parameters,
- $\overline{v^2}$ is the wall normal shear stress component,
- $T_s$ is the time scale.

The time scale is modeled as the maximum between the large-eddy times scale
and the small eddy or Kolmogorov time scale:

\begin{equation}
T_s = \max \left( \frac{k}{\epsilon}, 6.0 \sqrt{\frac{\mu}{\rho \epsilon}} \right) \,,
\end{equation}

where:

- $\epsilon$ is the dissipation rate of turbulent kinetic energy,
- $\mu$ is the dynamic molecular viscosity.

Finally, the square of the turbulent length scale is defined as follows:

\begin{equation}
L^2 = \left[ 0.23 \max ( L_{bulk}, L_{kolmogorov} ) \right] \,,
\end{equation}

where:

- $L_{bulk} = \frac{k^{1.5}}{\epsilon}$ is the bulk length scale,
- $L_{kolmogorov} = C_{\eta} \left( \frac{\mu^3}{\rho^3 \epsilon} \right)^{\frac{1}{4}}$ is the Kolmogorov length scale, with $C_{\eta} = 70$ being a closure parameter.

!alert note
We recommend using relaxation or dynamic relaxation (via decay rate of relaxation factors) for solving this equation.
The equation is an elliptic equation, which entails rapid propagation of information.
It is expected that this equation will be more numerically stiff than coupled equations for other turbulent variables.


!syntax parameters /FVKernels/INSFVTFSourceSink

!syntax inputs /FVKernels/INSFVTFSourceSink

!syntax children /FVKernels/INSFVTFSourceSink

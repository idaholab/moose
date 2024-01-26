# INSFVTV2SourceSink

The object computes the turbulent source and sink term for the wall normal stress component
of the Reynolds Stress Tensor, i.e., $\overline{v^2} = \overline{v'v'}$.

The net production minus destruction for the wall-normal stresses reads as follows:

\begin{equation}
\rho \left[ \min \left(k f, C_2 \frac{G_k}{\rho} - \delta_{v2} \right) - n \frac{\overline{v^2}}{T_s} \right] \,,
\end{equation}

where:

- $\rho$ is the fluid density,
- $k$ is the turbulent dynamic viscosity,
- $f$ is an elliptic relaxation function,
- $C_2 = 0.3$ is a model parameter,
- $G_k = \mu_t S^2$ is the production of turbulent kinetic energy with $\mu_t$ being the dynamic turbulent viscosity and $S$ is the shear strain tensor internal norm,
- $\delta_{v2}$ is the diffusivity function for the wall normal stress,
- $T_s$ is the time scale.

The diffusivity function for wall normal stresses is modeled as follows:

\begin{equation}
\frac{(C_1 - n) \overline{v^2} - \frac{2}{3} k (C_1 - 1.0)}{T_s} \,,
\end{equation}

where:

- $C_1 = 1.4$ and $n = 6.0$ are model parameters.

Finally, the time scale is modeled as the maximum between the large-eddy times scale
and the small eddy or Kolmogorov time scale:

\begin{equation}
\max \left( \frac{k}{\epsilon}, 6.0 \sqrt{\frac{\mu}{\rho \epsilon}} \right) \,,
\end{equation}

where:

- $\epsilon$ is the dissipation rate of turbulent kinetic energy,
- $\mu$ is the dynamic molecular viscosity.

!alert note
The v2f model is designed to handle near-wall damping wall effects in turbulent boundary layers
and to accommodate non-local effects, such as boundary layer detachment and reattachment phenomena.
The model is known to capture near-wall turbulence effects more accurately, which is beneficial for
accurate prediction of near-wall phenomena, such as skin friction, flow separation and reattachment, and heat transfer.

!alert note
The model for $\overline{v^2}$ should normally be coupled with a model for the elliptic relaxation function $f$.
The reason is that the $f$ function should determine the damping of wall normal stresses near the wall.
Although not recommended, the user may set the elliptic dissipation function to `1.0`
for modeling flow in free expansion jets.

!alert note
We recommend using relaxation for solving this equation.


!syntax parameters /FVKernels/INSFVTV2SourceSink

!syntax inputs /FVKernels/INSFVTV2SourceSink

!syntax children /FVKernels/INSFVTV2SourceSink

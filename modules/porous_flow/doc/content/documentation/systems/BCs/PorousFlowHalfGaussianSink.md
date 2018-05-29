# PorousFlowHalfGaussianSink

!syntax description /BCs/PorousFlowHalfGaussianSink

The basic sink $f(x,t)$ is scaled by a Gaussian flux multiplier of the pressure
of a fluid phase *or* the temperature $g(v)$:
\begin{equation*}
  s = f(t, x) \times g(v) \,
\end{equation*}
where
\begin{equation*}
  g(v) =
  \begin{cases}
    g_{\mathrm{max}} \exp((-0.5 (v - v_{\mathrm{center}}) / sd)^2), & \text{if}\ v < v_{\mathrm{center}} \\
    g_{\mathrm{max}},  & \text{otherwise}
  \end{cases}
\end{equation*}
Here the units of $f\times g$ are kg.m$^{-2}$.s$^{-1}$ (for fluids) or
J.m$^{-1}$.s$^{-1}$ (for heat). The parameters $g_{\mathrm{max}}$, $v_{\mathrm{center}}$ and $sd$ are given in the input file using the `max`, `center` and `sd` options, respectively.

If $f>0$ then the boundary condition will act as a sink, while if $f<0$ the boundary condition acts as a source.  If applied to a fluid-component equation, the function $f$ has units kg.m$^{-2}$.s$^{-1}$.  If applied to the heat equation, the function $f$ has units J.m$^{-2}$.s$^{-1}$.  These units are potentially modified if the extra building blocks enumerated below are used.

In addition, the sink may be multiplied by any or all of the following
quantities through the `optional parameters` list.

- Fluid relative permeability
- Fluid mobility ($k_{ij}n_{i}n_{j}k_{r} \rho / \nu$, where $n$ is the normal vector to the boundary)
- Fluid mass fraction
- Fluid internal energy
- Thermal conductivity

See [boundary conditions](boundaries.md) for many more details and discussion.

!syntax parameters /BCs/PorousFlowHalfGaussianSink

!syntax inputs /BCs/PorousFlowHalfGaussianSink

!syntax children /BCs/PorousFlowHalfGaussianSink

# PorousFlowHalfCubicSink

!syntax description /BCs/PorousFlowHalfCubicSink

The basic sink $f(x,t)$ is scaled by a cubic flux multiplier of the pressure
of a fluid phase *or* the temperature $g(v)$:
\begin{equation*}
  s = f(t, x) \times g(v) \,
\end{equation*}
where
\begin{equation*}
  g(v) =
  \begin{cases}
    g_{\mathrm{max}}, & \text{if}\ v < v - v_{\mathrm{center}} \\
    \frac{2 g_{\mathrm{max}} \left(2[v - v_{\mathrm{center}}] + v_{\mathrm{cutoff}}\right) (v - v_{\mathrm{center}} - v_{\mathrm{cutoff}})^2}{v_{\mathrm{cutoff}}^3},  & \text{otherwise}
  \end{cases}
\end{equation*}
Here the units of $f\times g$ are kg.m$^{-2}$.s$^{-1}$ (for fluids) or
J.m$^{-1}$.s$^{-1}$ (for heat). The parameters $g_{\mathrm{max}}$, $v_{\mathrm{center}}$ and $v_{\mathrm{cutoff}}$ are given in the input file using the `max`, `center` and `cutoff` options, respectively.

If $f>0$ then the boundary condition will act as a sink, while if $f<0$ the boundary condition acts as a source.  If applied to a fluid-component equation, the function $f$ has units kg.m$^{-2}$.s$^{-1}$.  If applied to the heat equation, the function $f$ has units J.m$^{-2}$.s$^{-1}$.  These units are potentially modified if the extra building blocks enumerated below are used.

In addition, the sink may be multiplied by any or all of the following
quantities through the `optional parameters` list.

- Fluid relative permeability
- Fluid mobility ($k_{ij}n_{i}n_{j}k_{r} \rho / \nu$, where $n$ is the normal vector to the boundary)
- Fluid mass fraction
- Fluid internal energy
- Thermal conductivity

See [boundary conditions](boundaries.md) for many more details and discussion.

!syntax parameters /BCs/PorousFlowHalfCubicSink

!syntax inputs /BCs/PorousFlowHalfCubicSink

!syntax children /BCs/PorousFlowHalfCubicSink

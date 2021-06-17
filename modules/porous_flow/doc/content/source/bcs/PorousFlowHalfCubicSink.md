# PorousFlowHalfCubicSink

!syntax description /BCs/PorousFlowHalfCubicSink

The basic sink $f(x,t)$ is scaled by a cubic flux multiplier of the pressure
of a fluid phase *or* the temperature.  Label the pressure or temperature by $v$.  Then the sink strength is
\begin{equation*}
  s = f(t, x) \times g(v) \ .
\end{equation*}

The function $g$ interpolates smoothly between $g(v_{\mathrm{cutoff}} + v_{\mathrm{center}}) = 0$ and $g(v_{\mathrm{center}}) = g_{\mathrm{max}}$.  Since a cubic interpolation is used, the function $g$ is continuous and its derivative is continuous everywhere.  Explicitly:
\begin{equation*}
  g(v) =
  \begin{cases}
    g_{\mathrm{max}} & \text{if}\ v > v_{\mathrm{center}} \\
    \frac{g_{\mathrm{max}} \left(2[v - v_{\mathrm{center}}] + v_{\mathrm{cutoff}}\right) (v - v_{\mathrm{center}} - v_{\mathrm{cutoff}})^2}{v_{\mathrm{cutoff}}^3},  & v_{\mathrm{cutoff}} + v_{\mathrm{center}} \leq v \leq v_{\mathrm{center}} \\
    0 & \text{if}\ v < v_{\mathrm{cutoff}} + v_{\mathrm{center}} \\
  \end{cases}
\end{equation*}
Here the units of $f\times g$ are kg.m$^{-2}$.s$^{-1}$ (for fluids) or
J.m$^{-1}$.s$^{-1}$ (for heat). The parameters $g_{\mathrm{max}}$, $v_{\mathrm{center}}$ and $v_{\mathrm{cutoff}}$ are given in the input file using the `max`, `center` and `cutoff` options, respectively.

This sink is often used in groundwater modelling of evapotranspiration, where:

- $v_{\mathrm{center}} = 0$, since evapotranspiration is maximum when the groundwater table is at or above the topography;
- $v_{\mathrm{cutoff}} = -10^{4}d$, where $d$ is the root depth at which there is no evapotransipiration from the groundwater system;
- $g_{\mathrm{max}}$ is the value of the pan evaporation, in kg.m$^{-2}$.s$^{-1}$.

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

# PorousFlowPiecewiseLinearSink

!syntax description /BCs/PorousFlowPiecewiseLinearSink

The basic sink $f(x,t)$ is multiplied by a piecewise linear MOOSE Function of the pressure
of a fluid phase $g(P^{\beta}-P_{\mathrm{e}})$ *or* the temperature $g(T-T_{\mathrm{e}})$:
\begin{equation*}
s = f(t, x) \times g(P^{\beta}-P_{\mathrm{e}}) \ \ \ \textrm{or}\ \ \ s = f(t, x)
\times g(T-T_{\mathrm{e}}) \ .
\end{equation*}
Here the units of $f\times g$ are kg.m$^{-2}$.s$^{-1}$ (for fluids) or
J.m$^{-1}$.s$^{-1}$ (for heat).

If $f>0$ then the boundary condition will act as a sink, while if $f<0$ the boundary condition acts as a source.  If applied to a fluid-component equation, the function $f$ has units kg.m$^{-2}$.s$^{-1}$.  If applied to the heat equation, the function $f$ has units J.m$^{-2}$.s$^{-1}$.  These units are potentially modified if the extra building blocks enumerated below are used.

In addition, the sink may be multiplied by any or all of the following
quantities through the `optional parameters` list.

- Fluid relative permeability
- Fluid mobility ($k_{ij}n_{i}n_{j}k_{r} \rho / \nu$, where $n$ is the normal vector to the boundary)
- Fluid mass fraction
- Fluid internal energy
- Thermal conductivity

See [boundary conditions](boundaries.md) for many more details and discussion.

!syntax parameters /BCs/PorousFlowPiecewiseLinearSink

!syntax inputs /BCs/PorousFlowPiecewiseLinearSink

!syntax children /BCs/PorousFlowPiecewiseLinearSink

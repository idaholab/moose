#Boundary conditions

MOOSE's Dirichlet and Neumann boundary conditions enable simulation of simple scenarios.
The Porous Flow module includes a very flexible boundary condition that allows many different
scenarios to be modelled. The boundary condition is built by adding various options to a basic
sink in the following way.

The basic sink is
\begin{equation}
s = f(t, x) \ ,
\end{equation}
where $f$ is a MOOSE Function of time and position on the boundary.

If $f>0$ then the boundary condition will act as a sink, while if $f<0$ the boundary
condition acts as a source.  If applied to a fluid-component equation, the function
$f$ has units kg.m$^{-2}$.s$^{-1}$.  If applied to the heat equation, the function
$f$ has units J.m$^{-2}$.s$^{-1}$.  These units are potentially modified if the extra
building blocks enumerated below are used.

This basic sink boundary condition is implemented in [`PorousFlowSink`](/porous_flow/PorousFlowSink.md).

The basic sink may be multiplied by a MOOSE Function of the pressure
of a fluid phase *or* the temperature:
\begin{equation}
s = f(t, x) \times g(P^{\beta}) \ \ \ \textrm{or}\ \ \ s = f(t, x)
\times g(T) \ .
\end{equation}
Here the units of $f\times g$ are kg.m$^{-2}$.s$^{-1}$ (for fluids) or
J.m$^{-1}$.s$^{-1}$ (for heat).  Some commonly use forms have been
hard-coded into the Porous Flow module for ease of use in simulations:

- A piecewise linear function (for simulating fluid or heat exchange with an external
  environment via a conductivity term, for instance), implemented in
  [`PorousFlowPiecewiseLinearSink`](/porous_flow/PorousFlowPiecewiseLinearSink.md)
- A half-gaussian (for simulating evapotranspiration, for
  instance), implemented in [`PorousFlowHalfGaussianSink`](/porous_flow/PorousFlowHalfGaussianSink.md)
- A half-cubic (for simulating evapotranspiration, for example), implemented in
  [`PorousFlowHalfCubicSink`](/porous_flow/PorousFlowHalfCubicSink.md)

In addition, the sink may be multiplied by any or all of the following
quantities

- Fluid relative permeability
- Fluid mobility ($k_{ij}n_{i}n_{j}k_{r} \rho / \nu$, where $n$ is the normal vector to the boundary)
- Fluid mass fraction
- Fluid internal energy
- Thermal conductivity

For example, a boundary condition corresponding to injection of fluid at a fixed temperature
could involve: (1) using a Dirichlet condition for temperature; (2) using $f=-1$ without any
multiplicative factors. More complicated examples with heat and fluid injection and production
are detailed in the test suite documentation.

Commonly, if fluid or heat is exiting the porous material, multiplication by relative permeaility,
mobility, mass fraction, enthalpy or thermal conductivity is necessary, while if fluid or heat is
entering the porous material this multiplication is not necessary.  Two sinks can be constructed
in a MOOSE input file: one involving *production* which is only active for $P>P_{\mathrm{e}}$ using a
Piecewise-linear sink (here $P_{\mathrm{e}}$ is the environmental pressure); and one involving
*injection*, which is only active for $P<P_{\mathrm{e}}$ using a piecewise-linear sink multiplied
by the appropriate factors.

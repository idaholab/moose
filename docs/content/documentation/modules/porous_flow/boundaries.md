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
s = f(t, x) \times g(P^{\beta} - P_{\mathrm{e}}) \ \ \ \textrm{or}\ \ \ s = f(t, x)
\times g(T - T_{\mathrm{e}}) \ .
\end{equation}
Here the units of $f\times g$ are kg.m$^{-2}$.s$^{-1}$ (for fluids) or
J.m$^{-1}$.s$^{-1}$ (for heat).  $P_{\mathrm{e}}$ and $T_{\mathrm{e}}$ are reference values, that
may be AuxVariables to allow spatial and temporal variance.  Some commonly use forms have been
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

## Example: fixing fluid porepressures using the PorousFlowSink

Frequently in PorousFlow simulations fixing fluid porepressures using
Dirichlet conditions is inappropriate.

  * Physically a Dirichlet condition corresponds to an environment
  outside a model providing a limitless source (or sink) of fluid and
  that does not happen in reality.
  * The Variables used may not be porepressure, so a straightforward
  translation to ``fixing porepressure'' is not easy.
  * The Dirichlet condition can lead to extremely poor convergence
  as it forces areas near the boundary close to unphysical or unlikely
  regions in parameter space.

It is advantageous to think of what the boundary condition
is physically trying to represent before using Dirichlet conditions,
and often a [`PorousFlowSink`](/porous_flow/PorousFlowSink.md) is more appropriate to use.
For instance, at the top of a groundwater-gas model, gas can freely
enter the model from the atmosphere, and escape the model to the
atmosphere, which may be modelled using a `PorousFlowSink`.  Water can
exit the model if the water porepressure increases above atmospheric
pressure, but the recharge if porepressure is lowered may be zero.
Again this may be modelled using a `PorousFlowSink`.

When fixing porepressures (or temperatures) using a `PorousFlowSink`,
what should the ``conductance'' be?  Consider an imaginary environment
that sits at distance $L$ from the model.  The imaginary environment
provides a limitless source or sink of fluid, but that fluid must
travel through the distance $L$ between the model and the imaginary
environment.  The flux of fluid from the model to this environment is
(approximately)
\begin{equation}
  \label{eq:fix.pp.bc}
  f = \frac{\rho k_{nn}
    k_{\mathrm{r}}}{\mu}\frac{P-P_{\mathrm{e}}}{L} \ .
\end{equation}
A similar equation holds for temperature (but in PorousFlow
simulations the heat is sometimes supplied by the fluid, in which case
the appropriate equation to use for the heat may be the above equation
multiplied by the enthalpy).  Here $k_{nn}$ is the permeability of the
region between the model and the imaginary environment,
$k_{\mathrm{r}}$ is the relative permeability in this region, $\rho$
and $\mu$ are the fluid density and viscosity.  The environment
porepressure is $P_{\mathrm{e}}$ and the boundary porepressure is
$P$.

If $L\sim 0$ this effectively fixes the porepressure to $P\sim
P_{\mathrm{e}}$, since $f$ is very large otherwise (physically:
fluid is rapidly removed or added to the system by the environment to
ensure $P=P_{\mathrm{e}}$).  If $L\sim\infty$ the boundary flux is
almost zero and does nothing.

Eq \eqref{eq:fix.pp.bc} (`Eq` `\eqref{eq:fix.pp.bc}`)Eqn~(\ref{}) may
be implemented in a number of ways.  A
[`PorousFlowPiecewiseLinearSink`](/porous_flow/PorousFlowPiecewiseLinearSink.md)
may be constructed that models \begin{equation} f = C (P -
P_{\mathrm{e}}) \ , \end{equation} that is, with `pt_vals = '-1E9
1E9'` and `multipliers = '-C C'` (the `1E9` is just an example: the
point is that it should be much greater than any expected
porepressure) and $P_{\mathrm{e}}$ provided by an AuxVariable (or set
to a constant value).  The numerical value of the conductance, $C$, is
$\rho k_{nn}k_{\mathrm{r}}/\mu/L$, must be set at an appropriate value
for the model.

Alternately, a
[`PorousFlowPiecewiseLinearSink`](/porous_flow/PorousFlowPiecewiseLinearSink.md)
may be constructed that has `use_mobility = true` and `use_relperm =
true`, and then $C = 1/L$.  This has three advantages: (1) the MOOSE
input file is simpler; (2) MOOSE automatically chooses the correct
mobility and relative permeability to use; (3) these parameters are
appropriate to the model so it reduces the potential for difficult
numerical situations occuring.

Finally, if $P_{\mathrm{e}}$ is varying over the boundary it must be constructed as an AuxVariable.  An alternative is to split the flux \begin{equation} f = CP - CP_{\mathrm{e}}\ .
\end{equation} Two PorousFlowSinks may be used.  The first term is a
`PorousFlowPiecewiseLinearSink` as just described (with
$P_{\mathrm{e}} = 0$).  The second term is a plain
[`PorousFlowSink`](/porous_flow/PorousFlowSink.md) with a
`flux_function` that is exactly equal to $P_{\mathrm{e}}$.

## Example: injection of fluid at a fixed temperature

A boundary condition corresponding to injection of fluid at a fixed temperature
could involve: (1) using a Dirichlet condition for temperature; (2) using $f=-1$ without any
multiplicative factors. More complicated examples with heat and fluid injection and production
are detailed in the test suite documentation.

## Fluids that both enter and exit the boundary

Commonly, if fluid or heat is exiting the porous material, multiplication by relative permeaility,
mobility, mass fraction, enthalpy or thermal conductivity is necessary, while if fluid or heat is
entering the porous material this multiplication is not necessary.  Two sinks can be constructed
in a MOOSE input file: one involving *production* which is only active for $P>P_{\mathrm{e}}$ using a
Piecewise-linear sink (here $P_{\mathrm{e}}$ is the environmental pressure); and one involving
*injection*, which is only active for $P<P_{\mathrm{e}}$ using a piecewise-linear sink multiplied
by the appropriate factors.

# Boundary conditions

MOOSE's Dirichlet and Neumann boundary conditions enable simulation of simple scenarios.
The Porous Flow module includes a very flexible boundary condition that allows many different
scenarios to be modelled. The boundary condition is built by adding various options to a basic
sink in the following way.

## Basic sink formulation

The basic sink is
\begin{equation}
s = f(t, x) \ ,
\end{equation}
where $f$ is a MOOSE Function of time and position on the boundary.

If $f>0$ then the boundary condition will act as a sink, while if
$f<0$ the boundary condition acts as a source.  If applied to a
fluid-component equation, the function $f$ has units
kg.m$^{-2}$.s$^{-1}$.  If applied to the heat equation, the function
$f$ has units J.m$^{-2}$.s$^{-1}$.  The units of $f$ are potentially
modified if the extra building blocks enumerated below are used (but
the units of the final result, $s$, will always be
kg.m$^{-2}$.s$^{-1}$ or J.m$^{-2}$.s$^{-1}$).

If the fluid flow through the boundary is required, use the `save_in`
command which will save the sink strength to an AuxVariable.  This
AuxVariable will be the flux (kg.s$^{-1}$ or J.s$^{-1}$) from each
node on the boundary, which is the product of $s$ and the area
attributed to that node.  If the total flux (kg.s$^{-1}$ or
J.s$^{-1}$) through the boundary is required, integrate the
AuxVariable over the boundary using a [`NodalSum`](NodalSum.md)
Postprocessor.

This basic sink boundary condition is implemented in [`PorousFlowSink`](PorousFlowSink.md).

## More elaborate options

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

- A piecewise linear function (for simulating fluid or heat exchange with an external environment via a conductivity term, for instance), implemented in [`PorousFlowPiecewiseLinearSink`](PorousFlowPiecewiseLinearSink.md)

- A half-gaussian (for simulating evapotranspiration, for instance), implemented in [`PorousFlowHalfGaussianSink`](PorousFlowHalfGaussianSink.md)

- A half-cubic (for simulating evapotranspiration, for example), implemented in [`PorousFlowHalfCubicSink`](PorousFlowHalfCubicSink.md)

In addition, the sink may be multiplied by any or all of the following
quantities

- Fluid relative permeability
- Fluid mobility ($k_{ij}n_{i}n_{j}k_{r} \rho / \mu$, where $n$ is the normal vector to the boundary)
- Fluid mass fraction
- Fluid internal energy
- Thermal conductivity

## Fixing fluid porepressures using the PorousFlowSink

Frequently in PorousFlow simulations fixing fluid porepressures using
Dirichlet conditions is inappropriate.

- Physically a Dirichlet condition corresponds to an environment
  outside a model providing a limitless source (or sink) of fluid and
  that does not happen in reality.
- The Variables used may not be porepressure, so a straightforward
  translation to ``fixing porepressure'' is not easy.
- The Dirichlet condition can lead to extremely poor convergence
  as it forces areas near the boundary close to unphysical or unlikely
  regions in parameter space.

It is advantageous to think of what the boundary condition
is physically trying to represent before using Dirichlet conditions,
and often a [`PorousFlowSink`](PorousFlowSink.md) is more appropriate to use.
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
  \label{eq:fix_pp_bc}
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

[eq:fix_pp_bc] may be implemented in a number of ways, the 2 most
common being the following.

A [`PorousFlowPiecewiseLinearSink`](PorousFlowPiecewiseLinearSink.md)
may be constructed that models
\begin{equation}
f = C (P -
P_{\mathrm{e}}) \ ,
\end{equation}
that is, with `pt_vals = '-1E9 1E9'`, `multipliers = '-1E9 1E9'`,
`PT_shift = Pe` and `flux_function = C`.  Here the `1E9` is just an
example: the point is that it should be much greater than any expected
porepressure) and $P_{\mathrm{e}}$ provided by an AuxVariable (or set
to a constant value).  The numerical value of the conductance, $C$, is
$\rho k_{nn}k_{\mathrm{r}}/\mu/L$, must be set at an appropriate value
for the model.

Alternately, a
[`PorousFlowPiecewiseLinearSink`](PorousFlowPiecewiseLinearSink.md)
may be constructed with the same `pt_vals`, `multipliers` and `PT_shift`, but with `use_mobility = true` and `use_relperm =
true`, and then $C = 1/L$.  This has three advantages: (1) the MOOSE
input file is simpler; (2) MOOSE automatically chooses the correct
mobility and relative permeability to use; (3) these parameters are
appropriate to the model so it reduces the potential for difficult
numerical situations occuring.

## Injection of fluid at a fixed temperature

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

## A 2-phase example

To illustrate that the "sink" part of the `PorousFlowSink` boundary condition works as planned, consider 2-phase flow along a line.  The simulation is initially saturated with water, and CO$_{2}$ is injected at the left-side.  The CO$_{2}$ front moves towards the right, but what boundary conditions should be placed on the right so that they don't interfere with the flow?

The answer is that two [`PorousFlowPiecewiseLinearSink`](PorousFlowPiecewiseLinearSink.md) are needed.  One pulls out water and the other pulls out CO$_{2}$.  They extract the fluids in proportion to their mass fractions, mobilities and relative permeabilities, in the way described above.  Let us explore the input file a little.

In this example there are two fluid components:

- fluid component 0 is water
- fluid component 1 is CO$_{2}$

and two phases:

- phase 0 is liquid
- phase 1 is gas

The water component only exists in the liquid phase and the CO$_{2}$ component only exists in the gaseous phase (the fluids are immiscible).  The Variables are the water and gas porepressures:

!listing modules/porous_flow/test/tests/sinks/injection_production_eg.i start=[Variables] end=[Kernels]

The `pwater` Variable is associated with the water component, while the `pgas` Variable is associated with the CO$_{2}$ component:

!listing modules/porous_flow/test/tests/sinks/injection_production_eg.i start=[Kernels] end=[UserObjects]

A van Genuchten capillary pressure is used

!listing modules/porous_flow/test/tests/sinks/injection_production_eg.i start=[./pc] end=[]

The remainder of the input file is pretty standard, save for the important `BCs` block:

!listing modules/porous_flow/test/tests/sinks/injection_production_eg.i start=[BCs] end=[Preconditioning]

Below are shown some outputs.  Evidently the boundary condition satisfies the requirements.

!media injection_production_eg_sg0.png style=width:50%;margin-left:10px caption=Gas saturation at breakthrough (the jaggedness of the line is because gas saturation is an elemental variable).  id=injection_production_eg_sg0

!media injection_production_eg_pp0.png style=width:50%;margin-left:10px caption=Porepressures at breakthrough.  id=injection_production_eg_pp0

!media injection_production_eg_sg1.png style=width:50%;margin-left:10px caption=Gas saturation at the end of simulation.  id=injection_production_eg_sg1

!media injection_production_eg_pp1.png style=width:50%;margin-left:10px caption=Porepressures at the end of simulation.  id=injection_production_eg_pp1

!media injection_production_eg.gif style=width:50%;margin-left:10px caption=Evolution of gas saturation (the jaggedness of the line is because gas saturation is an elemental variable).  id=injection_production_eg



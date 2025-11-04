# Boundary conditions

MOOSE's Dirichlet and Neumann boundary conditions enable simulation of simple scenarios.
The Porous Flow module includes flexible boundary conditions that allow many different
scenarios to be modelled. There are two classes of boundary conditions:

1. Those based on [PorousFlowSink](PorousFlowSink.md).  These are typically used to add or remove fluid or heat-energy through the boundary.  The basic sink adds/removes a fixed flux, but more elaborate sources/sinks add time-dependent fluxes, or fluxes dependent on fluid pressure or temperature, fluid mobility, enthalpy, etc.  These boundary conditions may also be used to control porepressure, temperature, or mass fractions on the boundary by adding/removing fluid or heat through interaction with an external environment.  It is often physically more correct and numerically advantageous to use these boundary conditions instead of [DirichletBC](DirichletBC.md).
2. Those based on [PorousFlowOutflowBC](PorousFlowOutflowBC.md), which is an "outflow" boundary condition that removes fluid components or heat energy as they flow to the boundary.  This models a "free" boundary that is "invisible" to the simulation.  Please see below for more description and warnings.

This page may be read in conjunction with the [description of some of the tests of the PorousFlow sinks](tests/sinks/sinks_tests.md).

## Class 1: Basic sink formulation

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

## Class 1: More elaborate options

The basic sink may be multiplied by a MOOSE Function of the pressure
of a fluid phase *or* the temperature:
\begin{equation}
  \label{eq:s_f_g}
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
- Fluid mobility ($k_{ij}n_{i}n_{j} \rho / \mu$, where $n$ is the normal vector to the boundary)
- Fluid mass fraction
- Fluid internal energy
- Thermal conductivity

## Class 1: Fixing fluid porepressures using the PorousFlowSink

Frequently in PorousFlow simulations fixing fluid porepressures using
Dirichlet conditions is inappropriate.

- Physically a Dirichlet condition corresponds to an environment
  outside a model providing a limitless source (or sink) of fluid and
  that does not happen in reality.
- The Variables used may not be porepressure, so a straightforward
  translation to "fixing porepressure" is not easy.
- The Dirichlet condition can lead to extremely poor convergence
  as it forces areas near the boundary close to unphysical or unlikely
  regions in parameter space.

### Physical Intuition Behind PorousFlowSink

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
what should the "conductance" be?  Consider an imaginary environment
that sits at distance $L$ from the model.  The imaginary environment
provides a limitless source or sink of fluid, but that fluid must
travel through the distance $L$ between the model and the imaginary
environment.  The flux of fluid from the model to this environment is
(approximately)
\begin{equation}
  \label{eq:fix_pp_bc}
  \frac{\rho k_{nn}
    k_{\mathrm{r}}}{\mu}\frac{P-P_{\mathrm{e}}}{L} \ .
\end{equation}
A similar equation holds for temperature (but in PorousFlow
simulations the heat is sometimes supplied by the fluid, in which case
the appropriate equation to use for the heat may be the above equation
multiplied by the enthalpy).  Here $k_{nn}$ is the permeability of the
region between the model and the imaginary environment
(which can also be thought of as the permeability of the adjacent element),
$k_{\mathrm{r}}$ is the relative permeability in this region, $\rho$
and $\mu$ are the fluid density and viscosity.  The environment
porepressure is $P_{\mathrm{e}}$ and the boundary porepressure is
$P$.

If $L\sim 0$ this effectively fixes the porepressure to $P\sim
P_{\mathrm{e}}$, since the flux is very large otherwise (physically:
fluid is rapidly removed or added to the system by the environment to
ensure $P=P_{\mathrm{e}}$).  If $L\sim\infty$ the boundary flux is
almost zero and does nothing.

### Numerical Implementation

The [`PorousFlowSink`](PorousFlowSink.md) is implemented in a fairly general way
that allows for flexibility in setting combinations of pressure and temperature boundary conditions. Due to its implementation,
it is difficult to draw a perfect analogy to the physical flux. Nevertheless,
[eq:fix_pp_bc] may be implemented in a number of ways, and one of the most
common involves a [`PorousFlowPiecewiseLinearSink`](PorousFlowPiecewiseLinearSink.md)
that follows the format of [eq:s_f_g] using $f(x,t)=C$ and $g(P-P_{\mathrm{e}})$ as a
piecewise linear function between ordered pairs of `pt_vals` (on the x-axis) and
`multiplier` (on the y-axis). An example of the function $g$ is shown in the figure below. It accepts $P-P_{\mathrm{e}}$ as an input and returns
a value that ends up multiplying $C$ to give a flux (as in [eq:s_f_g]). $C$ can be thought of as the conductance and is specified with `flux_function = C`. Its numerical value is discussed below. $P_{\mathrm{e}}$ can be specified using an AuxVariable or set to a constant value using `PT_shift = Pe`.


!media piecewiselinear_g_function.png style=width:50%;margin-left:10px caption=Depiction of $g(P-P_{\mathrm{e}})$ for PorousFlowPiecewiseLinearSink. The function accepts $P-P_{\mathrm{e}}$ as an input (i.e. the difference between a specified environment pressure and the pressure on the boundary element) and returns a value that multiplies $C$ to give the flux out of the domain. id=PiecewiseLinear_g_Function

To set a Dirichlet boundary condition $P = P_{\mathrm{e}}$, either $C$ or the slope of $g$ should be very large.
It is usually convenient to make the slope of $g$ equal to 1 by setting `pt_vals = '-1E9 1E9'` and `multipliers = '-1E9 1E9'`, and then $C$ can be selected appropriately.
The range for `pt_vals` should be sufficiently large that the simulation always occupies the region between the values specified (`-1E9 1E9` is a typical choice because porepressures encountered in many simulations are $O(10^6)$).
This ensures good convergence, and if in doubt set the `pt_vals` at a higher value than you expect.
If $P - P_{\mathrm{e}}$ falls outside of the range defined in `pt_vals`, then the slope of $g = 0$. This can be useful to set a boundary condition that will only allow for outflow (e.g. by using  `pt_vals = '0 1E9'`, `multipliers = '0 1E9'`).`

The numerical value of the conductance, $C$, is
$\rho k_{nn}k_{\mathrm{r}}/\mu/L$, must be set at an appropriate value
for the model.
Alternately, a
[`PorousFlowPiecewiseLinearSink`](PorousFlowPiecewiseLinearSink.md)
may be constructed with the same `pt_vals`, `multipliers` and `PT_shift`, but with `use_mobility = true` and `use_relperm =
true`, and then $C = 1/L$.  This has three advantages: (1) the MOOSE
input file is simpler; (2) MOOSE automatically chooses the correct
mobility and relative permeability to use; (3) these parameters are
appropriate to the model so it reduces the potential for difficult
numerical situations occurring.

Also note, if $C \times g$ is too large, the boundary residual will be much larger than residuals within the domain. This results in poor convergence.

So what value should be assigned to $C$? In the example below, $C = 10^{-5}$, $\rho \sim 10^3$ kg/m$^3$, $k = 10^{-15}$ m$^2$, $k_r = 1$, and $\mu \sim 10^{-3}$ Pa-s. Therefore $L \sim 10^{-4}$ m. This value of $L$ is small enough to ensure that the Dirichlet boundary condition is satisfied. If $C$ is increased to $10^{-2}$, $L \sim 10^{-7}$ m, and the simulation has difficulty converging. If $C = 10^{-11}$, $L\sim10^2$ m, and the boundary acts like a source of fluid from a distant reservoir (i.e. it no longer acts like a Dirichlet boundary condition).
The value of $C$ is simply $1/L$ if `use_mobility = true` and `use_relperm = true`.

!listing modules/porous_flow/test/tests/sinks/PorousFlowPiecewiseLinearSink_BC_eg1.i start=[BCs] end=[Postprocessors]

## Class 1: Injection of fluid at a fixed temperature

A boundary condition corresponding to injection of fluid at a fixed temperature
could involve: (1) using a Dirichlet condition for temperature; (2) using $f=-1$ without any
multiplicative factors. More complicated examples with heat and fluid injection and production
are detailed in the test suite documentation.

## Class 1: Fluids that both enter and exit the boundary

Commonly, if fluid or heat is exiting the porous material, multiplication by relative permeability,
mobility, mass fraction, enthalpy or thermal conductivity is necessary, while if fluid or heat is
entering the porous material this multiplication is not necessary.  Two sinks can be constructed
in a MOOSE input file: one involving *production* which is only active for $P>P_{\mathrm{e}}$ using a
Piecewise-linear sink (here $P_{\mathrm{e}}$ is the environmental pressure); and one involving
*injection*, which is only active for $P<P_{\mathrm{e}}$ using a piecewise-linear sink multiplied
by the appropriate factors.

## Class 1: A 2-phase example

To illustrate that the "sink" part of the `PorousFlowSink` boundary condition works as planned, consider 2-phase flow along a line.  (A similar problem using the [PorousFlowOutflowBCs](PorousFlowOutflowBC.md) is described below.)  The simulation is initially saturated with water, and CO$_{2}$ is injected at the left-side.  The CO$_{2}$ front moves towards the right, but what boundary conditions should be placed on the right so that they don't interfere with the flow?

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

!listing modules/porous_flow/test/tests/sinks/injection_production_eg.i start=[pc] end=[]

The remainder of the input file is pretty standard, save for the important `BCs` block:

!listing modules/porous_flow/test/tests/sinks/injection_production_eg.i start=[BCs] end=[Preconditioning]

Below are shown some outputs.  Evidently the boundary condition satisfies the requirements.

!media injection_production_eg_sg0.png style=width:50%;margin-left:10px caption=Gas saturation at breakthrough (the jaggedness of the line is because gas saturation is an elemental variable).  id=injection_production_eg_sg0

!media injection_production_eg_pp0.png style=width:50%;margin-left:10px caption=Porepressures at breakthrough.  id=injection_production_eg_pp0

!media injection_production_eg_sg1.png style=width:50%;margin-left:10px caption=Gas saturation at the end of simulation.  id=injection_production_eg_sg1

!media injection_production_eg_pp1.png style=width:50%;margin-left:10px caption=Porepressures at the end of simulation.  id=injection_production_eg_pp1

!media injection_production_eg.gif style=width:50%;margin-left:10px caption=Evolution of gas saturation (the jaggedness of the line is because gas saturation is an elemental variable).  id=injection_production_eg

## Class 2: Boundary terms for advection-diffusion equations

The theoretical background concerning boundary fluxes is developed in
this section.  The initial part of the presentation is general and not specific to Porous
Flow.  The variable $u$ may represent temperature, fluid mass,
mass-density of a species, number-density of a species, etc.  The flux
quantified below has units depending on the physical meaning of $u$:
if $u$ is temperature then the flux has units J.s$^{-1}$; if $u$ is
fluid mass then the flux has units kg.s$^{-1}$, etc.  For sake or
argument, $u$ is referred to as "fluid" in the discussion below.

### The advection equation

The advection equation for $u$ is
\begin{equation}
\frac{\partial u}{\partial t} = -\nabla (\mathbf{V} u) \ .
\end{equation}
The velocity field ${\mathbf{V}}$ advects $u$, meaning that $u$
follows the streamlines of ${\mathbf{V}}$.  The weak form of this equation reads
\begin{equation}
\label{eq:adv_eqn_weak}
0 = \int_{\Omega}\psi \frac{\partial u}{\partial t} - \int_{\Omega}\nabla\psi\cdot\mathbf{V}u + \int_{\partial\Omega}\psi \mathbf{n}\cdot\mathbf{V}u \ .
\end{equation}
In a MOOSE input file, the second term is typically represented by a
[`ConservativeAdvection`](ConservativeAdvection.md) `Kernel`, while
the final term is the flux out through the surface $\partial\Omega$
and can be represented by an `Outflow` or `Inflow` `BC`.

The flux out through an area $A$ is
\begin{equation}
\mathrm{flux}_{\mathrm{out}} = \int_{A} \mathbf{n}\cdot\mathbf{V} u \ .
\end{equation}
Here $\mathbf{n}$ is the outward normal to $A$.  If $A$ is an arbitrary internal area, this equation is always correct, but if $A$ is on the boundary of the simulation, the flux out is dictated by the boundary condition used.

There are various common boundary conditions.  For the sake of the arguments below,
imagine $\partial\Omega$ being an outside boundary of the MOOSE numerical model.

- If $\mathbf{n}\cdot\mathbf{V}>0$ then fluid is moving from the numerical model through the boundary towards the outside of the model.  If no BC is used then there is no term in the system of equations that is removing fluid from the boundary.  That is, no fluid is allowed out of the model through $\partial\Omega$, and fluid builds up at the boundary.  (Even though $\mathrm{flux}_{\mathrm{out}}=0$, $u$ does not have to be zero on the boundary.)

- If $\mathbf{n}\cdot\mathbf{V}>0$ then fluid is moving from the numerical model through the boundary towards the outside of the model.  An `Outflow` BC is exactly the final term in [eq:adv_eqn_weak].  This removes fluid through $\partial\Omega$ at exactly the rate specified by the advection equation, so this is a "free" boundary that is "invisible" to the simulation.

- If $\mathbf{n}\cdot\mathbf{V}<0$ then fluid is moving from outside the model through the boundary and into the model.  However, if no BC is used then no fluid enters the domain through $\partial\Omega$ (and $u$ will reduce).

- If $\mathbf{n}\cdot\mathbf{V}<0$ then fluid is moving from outside the model through the boundary and into the model.  An `Inflow` `BC`, which is exactly the last term in [eq:adv_eqn_weak] with $u=u_{B}$ fixed, may be used to specify an injection rate through this boundary ($u_{B}$ has units kg.s$^{-1}$.m$^{-2}$).


### The diffusion equation

The diffusion equation for $u$ (representing temperature, fluid mass,
mass-density of a species, etc) is
\begin{equation}
\frac{\partial u}{\partial t} = \nabla (D \nabla u) \ ,
\end{equation}
with weak form
\begin{equation}
\label{eq:diff_eqn_weak}
0 = \int_{\Omega}\psi \dot{u} + \int_{\Omega}\nabla\psi\cdot D\nabla u - \int_{\partial\Omega}\psi \mathbf{n}\cdot D\nabla u \ .
\end{equation}
The second is typically represented in MOOSE using the [`Diffusion`](source/kernels/Diffusion.md) Kernel, while the last term is the [`DiffusionFluxBC`](DiffusionFluxBC.md) BC (assuming $D=I$).  This last term is the flux out (kg.s$^{-1}$, or
J.s$^{-1}$, or whatever are appropriate units associated with $u$) through an area $\partial\Omega$.

The flux out through an arbitrary area $A$ is
\begin{equation}
\mathrm{flux}_{\mathrm{out}} = -\int_{A} \mathbf{n}\cdot D\nabla u \ .
\end{equation}
Here $\mathbf{n}$ is the outward normal to $A$.  If $A$ is an arbitrary internal area, this equation is always correct, but if $A$ is on the boundary of the simulation, the flux out is dictated by the boundary condition used.

Inclusion of a BC for $u$ in a MOOSE input file will specify something about the flux from the boundary.  Some examples are:

- A [`NeumannBC`](source/bcs/NeumannBC.md) simply adds a constant value (kg.s$^{-1}$.m$^{-2}$) to the Residual and integrates it over the boundary.  Adding this constant value corresponds to adding a constant flux of fluid and MOOSE will find the solution that has ${\mathbf{n}}\cdot D\nabla u = h$.  The value is the flux into the domain (negative of the out-going flux mentioned above).

- Using no BC at all on $\partial\Omega$ means that there is no boundary term in [eq:diff_eqn_weak] so no fluid gets removed from this boundary: it is impermeable.  MOOSE will find the solution that has ${\mathbf{n}}\cdot D\nabla u = 0$.

- Using a [`DirichletBC`](source/bcs/DirichletBC.md) fixes $u$ on the boundary.  The physical interpretation is that an external source or sink is providing or removing fluid at the correct rate so that $u$ remains fixed.

- Using a [`DiffusionFluxBC`](DiffusionFluxBC.md) will remove fluid at exactly the rate specified by the diffusion equation (assuming $D=I$: otherwise an extension to the DiffusionFluxBC class needs to be encoded into MOOSE).  This boundary condition is a "free" boundary that is "invisible" to the simulation.


### The advection-diffusion equation

The advection-diffusion equation for $u$ (representing temperature, fluid mass,
mass-density of a species, etc) is just a combination of the above:
\begin{equation}
\frac{\partial u}{\partial t} = \nabla (D \nabla u - \mathbf{V}u) \ ,
\end{equation}
with weak form
\begin{equation}
\label{eq:diff_adv_eqn_weak}
0 = \int_{\Omega}\psi \dot{u} + \int_{\Omega}\nabla\psi\cdot (D\nabla u - \mathbf{V}u) - \int_{\partial\Omega}\psi \mathbf{n}\cdot (D\nabla u - \mathbf{V}u) \ .
\end{equation}

The flux out (kg.s$^{-1}$, or
J.s$^{-1}$, or whatever are appropriate units associated with $u$) through an area
$A$ is
\begin{equation}
\mathrm{flux}_{\mathrm{out}} = -\int_{A} \mathbf{n}\cdot \left( D\nabla u - \mathbf{V}u\right) \ .
\end{equation}
Here $\mathbf{n}$ is the outward normal to $A$.

Inclusion of a BC for $u$ in a MOOSE input file will specify something about the flux from the boundary.  Some examples are.

- Using no BCs means the final term [eq:diff_adv_eqn_weak] is missing from the system of equations that MOOSE solves.  This means that no fluid is entering or exiting the domain from this boundary: the boundary is "impermeable".  The fluid will "pile up" at the boundary, if ${\mathbf{V}}$ is such that it is attempting to "blow" fluid out of the boundary (${\mathbf{n}}\cdot{\mathbf{V}}>0$).  Conversely, the fluid will deplete at the boundary, if ${\mathbf{V}}$ is attempting to "blow" fluid from the boundary into the model domain (${\mathbf{n}}\cdot{\mathbf{V}}<0$).

- A [`NeumannBC`](source/bcs/NeumannBC.md) adds a constant value (kg.s$^{-1}$.m$^{-2}$) to the Residual and integrates it over the boundary.  Adding this constant value corresponds to adding a constant flux of fluid and MOOSE will find the solution that has ${\mathbf{n}}\cdot (D\nabla u - \mathbf{V}u) = h$.  The value of $h$ is the flux into the domain (negative of the out-going flux mentioned above).

- Using an `OutflowBC` together with a [`DiffusionFluxBC`](DiffusionFluxBC.md) removes fluid through $\partial\Omega$ at exactly the rate specified by the advection-diffusion equation, so this is a "free" boundary that is "invisible" to the simulation.

### The PorousFlow mass flux

The [governing equation](porous_flow/governing_equations.md) for mass conservation of fluid species $\kappa$ is
\begin{equation}
0 = \frac{\partial M^{\kappa}}{\partial t} + M^{\kappa}\nabla\cdot{\mathbf
  v}_{s} + \nabla\cdot \mathbf{F}^{\kappa} + \Lambda M^{\kappa} - \phi I_{\mathrm{chem}} - q^{\kappa} \ .
\end{equation}
The standard [PorousFlow nomenclature](/porous_flow/nomenclature.md) as been used here.  The weak form is
\begin{equation}
\label{eqn.pf.mass.flux.weak}
0 = \int_{\Omega}\psi \dot{M}^{\kappa} + \int_{\Omega}\psi \left( M^{\kappa}\nabla\cdot{\mathbf v}_{s} + \Lambda M^{\kappa} - \phi I_{\mathrm{chem}} - q^{\kappa} \right) - \int_{\Omega}\nabla\psi\cdot \mathbf{F}^{\kappa} + \int_{\partial\Omega}\psi {\mathbf n}\cdot \mathbf{F}^{\kappa}
\end{equation}
The flux of species $\kappa$ out through an arbitrary area $A$ is
\begin{equation}
\mathrm{flux}_{\mathrm{out}} = \int_{A} \mathbf{n}\cdot\mathbf{F}^{\kappa} \ .
\end{equation}
This has SI units kg.s$^{-1}$.

Inclusion of a BC for porepressure or mass fraction in a MOOSE input file will specify something about the flux from the boundary.

- Using no BCs means the final term [eqn.pf.mass.flux.weak] is missing from the system of equations that MOOSE solves, in other words, it is zero.  Hence, no fluid is entering or exiting the domain from this boundary: the boundary is "impermeable", and MOOSE will find the solution $\mathbf{n}\cdot\mathbf{F}^{\kappa} = 0$.

- A [`NeumannBC`](source/bcs/NeumannBC.md) adds a constant value, $h$, (kg.s$^{-1}$.m$^{-2}$) to the Residual and integrates it over the boundary.  Adding this constant value corresponds to adding a constant flux of fluid species, and MOOSE will find the solution that has ${\mathbf{n}}\cdot \mathbf{F}^{\kappa} = -h$.  The value of $h$ is the flux into the domain (negative of the out-going flux mentioned above).

- Any of the [PorousFlowSink](PorousFlowSink.md) variants mentioned above act in the same way as the [NeumannBC](NeumannBC.md) by adding a value to the Residual and integrating it over the boundary, thereby allowing flux through the boundary to be specified.  However, the [PorousFlowSink](PorousFlowSink.md) variants are much more flexible than any of the NeumannBC variants that are coded into the MOOSE framework, so should be preferred in PorousFlow simulations.

- Fixing the porepressure or mass fraction with a [DirichletBC](DirichletBC.md) is also possible.  This corresponds to adding/removing fluid species to keep the porepressure or mass fraction fixed, which can lead to severe numerical problems (for instance, trying to remove a fluid species that has zero mass fraction) so DirichletBC should be used with caution in PorousFlow simulations.  Instead, one of the PorousFlowSink variants should usually be used, as described in detail above.

- Using a [PorousFlowOutflowBC](PorousFlowOutflowBC.md) removes fluid species through $\partial\Omega$ at exactly the rate specified by the Darcy equation [eqn.pf.mass.flux.weak], so this is a "free" boundary that is "invisible" to the simulation.   More details are given below.


### The PorousFlow heat flux

The [governing equation](porous_flow/governing_equations.md) for heat-energy conservation is
\begin{equation}
0 = \frac{\partial\mathcal{E}}{\partial t} + \mathcal{E}\nabla\cdot{\mathbf
  v}_{s} + \nabla\cdot \mathbf{F}^{T} -
\nu
  (1-\phi)\sigma^{\mathrm{eff}}_{ij}\frac{\partial}{\partial
    t}\epsilon_{ij}^{\mathrm{plastic}}
 - q^{T}
\end{equation}
The standard [PorousFlow nomenclature](/porous_flow/nomenclature.md) as been used here.  The weak form is
\begin{equation}
\label{eqn.pf.heat.flux.weak}
0 = \int_{\Omega}\psi \dot{\mathcal{E}}^{\kappa} + \int_{\Omega}\psi \left( \mathcal{E}\nabla\cdot{\mathbf v}_{s} -
\nu
  (1-\phi)\sigma^{\mathrm{eff}}_{ij}\frac{\partial}{\partial
    t}\epsilon_{ij}^{\mathrm{plastic}}
 - q^{T} \right) - \int_{\Omega}\nabla\psi\cdot \mathbf{F}^{T} + \int_{\partial\Omega}\psi {\mathbf n}\cdot \mathbf{F}^{T}
\end{equation}
The flux of heat energy out through an arbitrary area $A$ is
\begin{equation}
\mathrm{flux}_{\mathrm{out}} = \int_{A} \mathbf{n}\cdot\mathbf{F}^{T} \ .
\end{equation}
This has SI units J.s$^{-1}$.  Analogous remarks to the mass-flow case can be made concerning possible different types of BC.

## Class 2: The PorousFlowOutflowBC

The [PorousFlowOutflowBC](PorousFlowOutflowBC.md) adds the following term to the residual
\begin{equation}
\int_{\partial\Omega}\psi {\mathbf n}\cdot \mathbf{F}
\end{equation}
Various forms of $\mathbf{F}$ may be chosen, as discussed in the [PorousFlowOutflowBC documentation](PorousFlowOutflowBC.md), so that this BC removes fluid species or heat energy through $\partial\Omega$ at exactly the rate specified by the Darcy equation [eqn.pf.mass.flux.weak] or the heat equation [eqn.pf.heat.flux.weak].  Therefore, this BC can be used to represent a "free" boundary through which fluid or heat can freely flow: the boundary is "invisible" to the simulation.

!alert note
Ensure your normal vector points *out* of the model, otherwise your fluxes will have the wrong sign.

!alert warning
`PorousFlowOutflowBC` does not model the interface of the model with "empty space".  Imagine a model of a porous-media pipe containing water.  `PorousFlowOutflowBC` does *not* model the situation where the pipe has an end through which the water flows into empty space.  Instead, `PorousFlowOutflowBC` allows only part of the pipe to be modelled: when water exits the model it continues freely into the unmodelled section of the pipe.  In this sense, the model's boundary is "invisible" to the simulation.  This has a further consequence: if there is a sink in the modelled section, `PorousFlowOutflowBC` will allow water to flow from the unmodelled section into the modelled section.

!alert note
The rate of outflow is limited by the permeability, the viscosity of the fluid, etc, in exactly the same way that the Darcy velocity is limited by these quantities.  This means, for instance, if you inject a lot of fluid or heat into the model, it will take the `PorousFlowOutflowBC` some time to "suck" it all out.


### Example: a 2-phase model

This example is similar to the 2-phase model employing a set of [PorousFlowPiecewiseLinearSinks](PorousFlowPiecewiseLinearSink.md) described above.  Consider 2-phase flow along a line.  This time, both water and CO$_{2}$ are injected from the left boundary, while the right boundary has two [PorousFlowOutflowBCs](PorousFlowOutflowBC.md).

In this example there are two fluid components:

- fluid component 0 is water
- fluid component 1 is CO$_{2}$

and two phases:

- phase 0 is liquid
- phase 1 is gas

The water component only exists in the liquid phase and the CO$_{2}$ component only exists in the gaseous phase (the fluids are immiscible).  The Variables are the water and gas porepressures:

!listing modules/porous_flow/test/tests/sinks/injection_production_eg_outflowBC.i start=[Variables] end=[Kernels]

The `pwater` Variable is associated with the water component, while the `pgas` Variable is associated with the CO$_{2}$ component:

!listing modules/porous_flow/test/tests/sinks/injection_production_eg_outflowBC.i start=[Kernels] end=[UserObjects]

The remainder of the input file is pretty standard, save for the important `BCs` block, where it is seen that water is injected with a rate of $0.01\,$g.s$^{-1}$ and CO$_{2}$ at a rate of $0.02\,$g.s$^{-1}$, and water and CO$_{2}$ are allowed to freely exit from the model using the [PorousFlowOutflowBCs](PorousFlowOutflowBC.md):

!listing modules/porous_flow/test/tests/sinks/injection_production_eg_outflowBC.i block=BCs

The outflow rates are plotted in [ip_outflowBC], where it can be seen that the asymptotic values are correct.

!media sinks/ip_outflowBC.png
	style=width:60%;margin:auto;padding-top:2.5%;
	id=ip_outflowBC
	caption=Mass flow rates from the two-phase model that contains a 0.01g/s source of water and a 0.02g/s source of gas, as well as PorousFlowOutflowBC boundary conditions that allow the fluids to freely exit the model.


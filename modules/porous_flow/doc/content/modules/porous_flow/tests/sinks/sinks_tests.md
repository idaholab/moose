# Sinks test descriptions

## Introduction

As described in [the page on boundary conditions](boundaries.md), a number of different sink boundary conditions have been implemented
in PorousFlow.  This page describes some of the tests of the various forms of sink/source boundary conditions.

To make these into sources instead of sinks, the
strength of the flux just needs to be made negative.  All the sinks
are implemented using full upwinding.  This is to prevent the sink
from attempting to remove fluid from a node that actually contains no
fluid.

The basic sink uses a Function to specify the flux on the boundary,
and also has the option of multiplying by any combination of: the
fluid mobility, the relative permeability, or a mass fraction.  These
latter multiplying factors are all useful in the case of sinks to
prevent an unlimited amount of fluid being withdrawn from the porous
medium, which can lead to extremely poor nonlinear convergence even if
only one node in the entire mesh is "running dry".

Derived from the basic one, is another boundary condition that allows
the flux to be modified by a piecewise-linear function of
porepressure, which is useful for the case where transfer coefficients
are defined across the boundary, or more complicated situations.

Also derived from the basic one are two others, in which the flux is
governed by a half Gaussian or half cubic function of porepressure,
which are useful for modelling evapotranspiration through a boundary.

## Basic PorousFlow Sink

### Test 1

A sink flux of strength 6$\,$kg.m$^{-2}$.s$^{-1}$ is applied to the left
edge ($x=0$) of a 3D mesh.  A single-phase, single-component fluid is
used, and the porepressure is initialised to $p=y+1$ (for $0\leq y\leq
1$).  No fluid flow within the element is used, so the masses of fluid
at the finite-element nodes behave independently.  The fluid is
assumed to have density $\rho = 1.1 \exp(p/1.3)\,$kg.m$^{-3}$.  The
porosity is 0.1.

The input file:

!listing modules/porous_flow/test/tests/sinks/s01.i

Under these conditions, assuming $p\geq 0$ so that the porous medium
is fully saturated with fluid, the fluid mass at a node should obey
\begin{equation}
m = V\phi\rho = V\times0.1\times 1.1\exp(p/1.3) = m(t=0) - 6At \ ,
\end{equation}
where $V$ is the volume occupied by the node, and $A$ is its area
exposed to the flux.  MOOSE correctly produces this result, as
illustrated in [s01_fig].

!media sinks/s01.png style=width:50%;margin-left:10px caption=Results of Test 1, illustrating that MOOSE correctly applies a constant sink flux to boundary nodes. id=s01_fig

### Test 2

An identical setup to Test 1 is used here, but with the sink flux
strength being multiplied by the mobility:
\begin{equation}
\mathrm{mobility} = n_{i}k_{ij}n_{j}\rho/\nu \ ,
\end{equation}
where $n_{i}k_{ij}n_{j}$ is the permeability tensor $k$ projected onto
the normal direction to the boundary $n$, and the fluid density and
viscosity are $\rho$ and $\nu$, respectively.  In this example
$\nu=1.1\,$Pa.s and $n_{i}k_{ij}n_{j}=0.2\,$m$^{2}$.  The other
parameters are the same as Test 1, except now the strength of the flux
is 6$\,$Pa.s$^{-1}$.

The input file:

!listing modules/porous_flow/test/tests/sinks/s02.i

In this case, the expected result is (for $p>0$)
\begin{equation}
\frac{\mathrm{d} m}{\mathrm{d} t} = V\phi \frac{\mathrm{d} \rho}{\mathrm{d} t} = -6 A
\frac{n_{i}k_{ij}n_{j}\rho}{\mu} \ .
\end{equation}
MOOSE correctly produces this result, as illustrated in [s02_fig].

!media sinks/s02.png style=width:50%;margin-left:10px caption=Results of Test 2, illustrating that MOOSE correctly applies a constant sink flux modified by the fluid mobility.  (A slight drift away from the expected result is due to MOOSE taking large time steps.) id=s02_fig

### Test 3

An identical setup to Test 1 is used here, but with the sink flux
strength being multiplied by the relative permeability, which is
chosen to be:
\begin{equation}
\kappa_{\mathrm{rel}} = S^{2} \ ,
\end{equation}
with $S$ being the fluid saturation.  A van Genuchten capillary
relationship is used:
\begin{equation}
S = \left( 1 + (-\alpha p)^{1/(1-m)} \right)^{-m} \ ,
\end{equation}
with $\alpha = 1.1\,$Pa$^{-1}$, and $m=0.5$.  The porepressure is
initialised to be $p=-y$.  The other parameters are
identical to Test 1.

The input file:

!listing modules/porous_flow/test/tests/sinks/s02.i

In this case, the expected result is
\begin{equation}
\frac{\mathrm{d} m}{\mathrm{d} t} = V\phi \frac{\mathrm{d} \rho S}{\mathrm{d} t} = -6 A S^{2} \ .
\end{equation}
MOOSE correctly produces this result, as illustrated in [s03_fig].

!media sinks/s03.png style=width:50%;margin-left:10px caption=Results of Test 3, illustrating that MOOSE correctly applies a constant sink flux modified by the fluid relative permeability. id=s03_fig

### Test 4

A similar setup to Test 1 is used here, but with a 3-component,
single-phase fluid, with the sink flux only extracting the second
component, with a rate proportional to the mass fraction of that
component.

The input file:

!listing modules/porous_flow/test/tests/sinks/s07.i

This test checks that the flux is correctly implemented (see [s07_fig]) and that the correct fluid component is being
withdrawn from the correct nodes.  MOOSE produces the expected result.

!media sinks/s07.png style=width:50%;margin-left:10px caption=Results of Test 4, illustrating that MOOSE correctly applies a sink flux of a particular fluid component proportional to the component's mass fraction. id=s07_fig

### Test 5

A sink is applied to the left edge ($x=0$) of a 3D mesh.  A
3-component, 2-phase fluid is used.  Call the two phases "water" and
"gas".  The porepressures are initialised to $p_{\mathrm{water}} =
y$ and $p_{\mathrm{gas}} = y + 3$.  The mass fractions are initialised
to $(0.3, 0.35, 0.35)$ in the water phase, and $(0.1, 0.8, 0.1)$ in
the gas phase.  The water phase is assumed to have density
$\rho_{\mathrm{water}} = 1.5 \exp(p_{\mathrm{water}}/2.3)$, and the gas phase
$\rho_{\mathrm{gas}} = 1.1 \exp(p_{\mathrm{gas}}/1.3)$.  A van Genuchten capillary
relationship is used:
\begin{equation}
S_{\mathrm{water}} = \left( 1 + (\alpha (p_{\mathrm{gas}} - p_{\mathrm{water}})^{1/(1-m)} \right)^{-m} \ ,
\end{equation}
with $\alpha = 1.1\,$Pa$^{-1}$, and $m=0.5$.  The water relative
permeaility is assumed to be Corey type with exponent $1$, and the gas
phase has exponent $2$ (that is $\kappa_{\mathrm{rel,gas}} =
S_{\mathrm{gas}}^{2}$, with $S_{\mathrm{gas}} = 1 -
S_{\mathrm{water}}$).

The sink flux acts only on the second component.  It is multiplied by the
relative permeability of the gas phase, and the mass fraction of the
second component in the gas phase.  This is possibly meaningless
physically, but acts as a good test of the PorousFlowSink.  In this
test the mass fractions remain fixed: there is nothing to induce a
change of a component from the water phase to the gas phase since the
only Kernels used are mass-conservation Kernels that simply demand
mass conservation of each fluid component (summed over each phase).

The input file:

!listing modules/porous_flow/test/tests/sinks/s08.i

The test checks whether MOOSE is correctly applying the sink flux, and
that the fluid-component masses at the nodes respond correctly to the
flux.  [s08_fig] demonstrates that MOOSE produces the
expected result.

!media sinks/s08.png style=width:50%;margin-left:10px caption=Results of Test 5, illustrating that in a 2-phase system MOOSE correctly applies a sink flux of a particular fluid component proportional to the component's mass fraction and the relative permeaility of the gas phase. id=s08_fig


## Piecewise-linear sink

A sink flux of strength
\begin{equation}
f = \left\{
\begin{array}{ll}
8 & \mathrm{\ if\ } p > 0.8 \\
8(p + 0.2) & \mathrm{\ if\ } 0.3 \leq p \leq 0.8 \\
4 & \mathrm{\ if\ } p < 0.3 \ ,
\end{array}
\right.
\end{equation}
(measured in kg.m$^{-2}$.s$^{-1}$) is applied to the right side
($x=1$) of a 3D mesh.  A single-phase, single-component fluid is used,
and the porepressure is initialised to $p=y+1$ (for $0\leq y \leq 1$).
No fluid flow within the element is used, so the masses of fluid at
the finite-element nodes behave independently.  The fluid is assumed
to have density $\rho = 1.1 \exp(p/1.3)\,$kg.m$^{-3}$.  The porosity
is 0.1.

The input file:

!listing modules/porous_flow/test/tests/sinks/s04.i

Under these conditions, the expected result for the fluid mass at a
node on the right side of the mesh is
\begin{equation}
\frac{\mathrm{d} m}{\mathrm{d} t} = V\phi \frac{\mathrm{d} \rho S}{\mathrm{d} t} = -f A \ .
\end{equation}
The notation is the same as in previous sections.

The test checks that the mass evolves according to this equation, and
that the flux is applied correctly.  [s04_fig] demonstrates
agreement with the expected flux and the MOOSE implementation.

!media sinks/s04.png style=width:50%;margin-left:10px caption=A piecewise-linear sink flux is correctly modelled by MOOSE. id=s04_fig


## Half-Gaussian sink

A sink flux of strength
\begin{equation}
f = \left\{
\begin{array}{ll}
6 & \mathrm{\ if\ } p \geq 0.9 \\
6\exp\left(-\frac{1}{2} \left(\frac{p-0.9}{0.5} \right)^{2}\right) & \mathrm{\ if\ } p < 0.9 \ .
\end{array}
\right.
\end{equation}
(measured in kg.m$^{-2}$.s$^{-1}$) is applied to the right side
($x=1$) of a 3D mesh.  This is a half-Gaussian sink with center
0.9$\,$Pa, standard deviation 0.5$\,$Pa and maximum 6.  A single-phase,
single-component fluid is used, and the porepressure is initialised to
$p=y+1.4$ (for $0\leq y \leq 1$).  No fluid flow within the element is
used, so the masses of fluid at the finite-element nodes behave
independently.  The fluid is assumed to have density $\rho = 1.1
\exp(p/1.3)\,$kg.m$^{-3}$.  The porosity is 0.1.  A van Genuchten capillary
relationship is used:
\begin{equation}
S = \left( 1 + (-\alpha p)^{1/(1-m)} \right)^{-m} \ ,
\end{equation}
with $\alpha = 1.1$\,Pa$^{-1}$, and $m=0.5$.

The input file:

!listing modules/porous_flow/test/tests/sinks/s05.i

Under these conditions, the expected result for the fluid mass at a
node on the right side of the mesh is
\begin{equation}
\frac{\mathrm{d} m}{\mathrm{d} t} = V\phi \frac{\mathrm{d} \rho S}{\mathrm{d} t} = -f A \ .
\end{equation}
The notation is the same as in previous sections.

The test checks that the mass evolves according to this equation, and
that the flux is applied correctly.  [s05_fig] demonstrates
agreement with the expected flux and the MOOSE implementation.

!media sinks/s05.png style=width:50%;margin-left:10px caption=A half-Gaussian sink flux with center 0.9Pa and standard
  deviation 0.5Pa is correctly modelled by MOOSE.  id=s05_fig


## Half-cubic sink

A sink flux of strength
\begin{equation}
f = \left\{
\begin{array}{ll}
3 & \mathrm{\ if\ } p \geq 0.9 \\
\frac{3}{(-0.8)^3} (2(p-0.9) + 0.8) (p - 0.9 + 0.8)^2 & \mathrm{\ if\ }
0.1 < p < 0.9 \\
0 & \mathrm{\ if\ } p \leq 0.1
\end{array}
\right.
\end{equation}
(measured in kg.m$^{-2}$.s$^{-1}$) is applied to the right side
($x=1$) of a 3D mesh.  This is a half-cubic sink with center
0.9$\,$Pa, cutoff $-0.8\,$Pa, and maximum 3$\,$kg.m$^{-2}$.s$^{-1}$.  A
single-phase, single-component fluid is used, and the porepressure is
initialised to $p=x(y+1)$ (for $0\leq y \leq 1$ and $0\leq x \leq 1$).
No fluid flow within the element is used, so the masses of fluid at
the finite-element nodes behave independently.  The fluid is assumed
to have density $\rho = 1.1 \exp(p/1.3)\,$kg.m$^{-3}$.  The porosity
is 0.1.

The input file:

!listing modules/porous_flow/test/tests/sinks/s06.i

Under these conditions, the expected result for the fluid mass at a
node on the right side of the mesh is
\begin{equation}
\frac{\mathrm{d} m}{\mathrm{d} t} = V\phi \frac{\mathrm{d} \rho S}{\mathrm{d} t} = -f A \ .
\end{equation}
The notation is the same as in previous sections.

The test checks that the mass evolves according to this equation, and
that the flux is applied correctly.  [s06_fig] demonstrates
agreement with the expected flux and the MOOSE implementation.

!media sinks/s06.png style=width:50%;margin-left:10px caption=A half-cubic sink with center 0.9Pa, cutoff -0.8Pa, and
  maximum 3kg/m2/s is correctly modelled by MOOSE.  id=s06_fig

## Piecewise linear sink, and advection of a fluid component

The porepressure at a boundary may be maintained at a fixed value by
applying a sufficiently strong piecewise-linear sink:
\begin{equation}
f = Cp \ ,
\end{equation}
(measured in kg.m$^{-2}$.s$^{-1}$) for large conductance $C$.  Note that if $C$ is too
  large then it will dominate the numerics and MOOSE will not converge.

Similarly, for the multi-component case, the flux
of of fluid component $\kappa$ should be made proportional to the
component mass fraction, $\chi^{\kappa}$:
\begin{equation}
f^{\kappa} = C\chi^{\kappa}p \ .
\label{eqn_flux_rhs}
\end{equation}
This is a "natural" boundary condition, in that fluid exits or
enters the porous material at a rate dictated by the mass-fraction
within the porous material.  This means, for instance, that if fluid
is exiting ($p>0$ in this case) then only components that exist at the
boundary system will exit, and MOOSE will not attempt to extract fluid
components that have zero mass-fraction.

This example concerns a 1D porous material occupying the space $0\leq
x \leq 1$.  It contains a single phase fluid with two
fluid components.  The porous material initially only contains fluid
component $1$, and there is a pressure gradient:
\begin{equation}
p(t=0) = 1 - x \ \ \ \mathrm{and}\ \ \ \chi^{0}(t=0) = 0 \ .
\end{equation}
For $t>0$, fluid component $0$ is introduced on the material's left
side ($x=0$), by applying the fixed boundary conditions:
\begin{equation}
p(x=0) = 1 \ \ \ \mathrm{and}\ \ \ \chi^{0}(x=0) = 1 \ .
\end{equation}
The right-hand side, at $x=1$, is subjected to the flux of
[eqn_flux_rhs].  The fluid-component $0$ flows from the left
side to the right side via the pressure gradient.  To simplify the
following analysis, the fluid bulk modulus is taken to be very large.

The input file:

!listing modules/porous_flow/test/tests/sinks/s09.i

Because the fluid bulk modulus is very large, $\partial P/\partial x = -1$ is a
solution for all time.  This means that the governing equation reduces
to
\begin{equation}
\phi \frac{\partial \chi}{\partial t} =  (k_{ij}/\mu) \nabla_{j}
\chi \nabla_{i}P \ .
\end{equation}
In this equation $\phi$ is the porosity, $k_{ij}$ is the permeability
tensor, and $\mu$ is the fluid viscosity.  This is just the advection
equation for the mass fraction $\chi$, with velocity
\begin{equation}
\mathrm{velocity}_{j} = \nabla_{i}P\frac{k_{ij}}{\mu\phi} \ .
\end{equation}
In this test, the parameters are chosen such that velocity$=1\,$m.s$^{-1}$.

The sharp front (described by the advection equation
with the initial and boundary conditions) is not maintained by
MOOSE.  This is due to numerical diffusion, which is particularly
strong in the upwinding scheme used in this test.  For many more details, see the [stabilization page](stabilization.md).
Nevertheless, MOOSE advects the smooth front with the correct
velocity, as shown in [s09_fig].

The sharp front is not maintained by MOOSE even when no
upwinding is used.  In the case at hand, which uses a fully-saturated
single-phase fluid, the [FullySaturated](PorousFlowFullySaturatedDarcyFlow.md) versions of the Kernels
may be used in order to compare with the standard fully-upwinded
Kernels.  These Kernels do not employ any upwinding
whatsoever, so less numerical
diffusion is expected.  This is demonstrated in [s09_fig].  Two additional points may also be
nocied: (1) the lack of upwinding has produced a "bump" in the
mass-fraction profile near the concentrated side; (2) the lack of upwinding
means the temperature profile moves slightly slower than it should.
These two affects reduce as the mesh density is increased, however.

The input file using the fully-saturated Kernels:

!listing modules/porous_flow/test/tests/sinks/s09_fully_saturated.i

!media sinks/s09.png style=width:80%;margin-left:10px caption=Results of the advection of a fluid component test, illustrating that the numerical implementation of porous flow within MOOSE diffuses sharp fronts, but advects them at the correct velocity (which is 1m/s in this case).  Notice the centre of the front is at the correct position in each picture.  Less diffusion is experienced when upwinding is not used, but notice the slight "bump" in the non-upwinded version at early times.  id=s09_fig

## PorousFlowOutflowBC and advection of a fluid component

The same problem as described in the previous section may also be modelled using a [PorousFlowOutflowBC](PorousFlowOutflowBC.md) on the right-hand boundary to allow `component = 1` to exit the system:

!listing sinks/s13.i block=BCs

Again the sharp front (described by the advection equation
with the initial and boundary conditions) is not maintained by the fully-upwind stabilization used.  For many more details, see the [stabilization page](stabilization.md).
Nevertheless, MOOSE advects the smooth front with the correct
velocity, as shown in [s09_fig], and the PorousFlowOutflowBC allows the fluid to freely exit the right-hand boundary.

!media sinks/s13.png style=width:80%;margin-left:10px caption=Results of the advection of a fluid component test utilizing a PorousFlowOutflowBC on the right-hand side to allow the fluid to flow freely from that boundary.  id=s13_fig

## PorousFlowOutflowBC recording fluid flow rates through a boundary

A [PorousFlowPointSourceFromPostprocessor](PorousFlowPointSourceFromPostprocessor.md) injects 1$\,$kg.s$^{-1}$ into a 2D model containing a fully-saturated, single-component fluid:

!listing sinks/s14.i block=DiracKernels

[PorousFlowOutflowBCs](PorousFlowOutflowBC.md) are applied to all the outer boundaries to allow the fluid to escape freely from the model:

!listing sinks/s14.i block=BCs

The outflow at each node is recorded into the `nodal_outflow` AuxVariable, and the total outflow is recorded using a [NodalSum](NodalSum.md) postprocessor:

!listing sinks/s14.i block=Postprocessors

The results are shown in [s14_fig] where it is clear that the total outflow approaches the expected value of 1$\,$kg.s$^{-1}$.

!media sinks/s14.png style=width:80%;margin-left:10px caption=Total flow rate from a model that includes a 1kg/s source of fluid.  id=s14_fig

## PorousFlowOutflowBC recording heat flow rates through a boundary

A thermo-hydro simulation involving a fully-saturated, single-component fluid coupled with temperature is considered here:

!listing sinks/s15.i block=PorousFlowFullySaturated

A [PorousFlowPointSourceFromPostprocessor](PorousFlowPointSourceFromPostprocessor.md) injects 1$\,$J.s$^{-1}$ into the model:

!listing sinks/s15.i block=DiracKernels

[PorousFlowOutflowBCs](PorousFlowOutflowBC.md) with `flux_type = heat` and `variable = T` allows heat-energy to escape freely from the model:

!listing sinks/s15.i block=BCs

The outflow at each node is recorded into the `nodal_outflow` AuxVariable, and the total outflow is recorded using a [NodalSum](NodalSum.md) postprocessor:

!listing sinks/s15.i block=Postprocessors

The results are shown in [s15_fig] where it is clear that the total outflow approaches the expected value of 1$\,$J.s$^{-1}$.

!media sinks/s15.png style=width:80%;margin-left:10px caption=Total flow rate from a model that includes a 1J/s source of fluid.  id=s15_fig


## PorousFlowOutflowBC allowing fluid to exit a 2-phase, 2-component system

In a 2-phase, 2-component system, the components are allowed to freely exit the system with using two [PorousFlowOutflowBCs](PorousFlowOutflowBC.md):

!listing modules/porous_flow/test/tests/sinks/injection_production_eg_outflowBC.i block=BCs

This test checks that the BCs remove fluid at the correct rate.  The verification is plotted in [ip_outflowBC], where it can be seen that the asymptotic values are correct.

!media sinks/ip_outflowBC.png
	style=width:60%;margin:auto;padding-top:2.5%;
	id=ip_outflowBC
	caption=Mass flow rates from the two-phase model that contains a 0.01g/s source of water and a 0.02g/s source of gas, as well as PorousFlowOutflowBC boundary conditions that allow the fluids to freely exit the model.


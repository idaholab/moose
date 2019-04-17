# Point and line sources and sinks

This page may be read in conjunction with the [theory behind Dirac Kernels in PorousFlow](sinks.md).

## Geometric tests

The test suite contains many core tests that demonstrate:

- when a point sink is placed at a node, it withdraws fluid (or heat) from only that node, for instance

!listing modules/porous_flow/test/tests/dirackernels/pls01.i


- when a point sink that is proportional to mobility (or relative permeability, etc) is placed in an element where some nodes have zero mobility (or relative permeability, etc), then fluid (or heat) is not extracted from those nodes, for instance

!listing modules/porous_flow/test/tests/dirackernels/pls03.i

## Basic point sources/sinks

The following input file extracts fluid at the rate (kg.s$^{-1}$)
\begin{equation}
\mathrm{source} = \left\{
\begin{array}{ll}
-0.1 & \mathrm{\ for\ } 100 \leq t \leq 300 \\
-0.1 & \mathrm{\ for\ } 600 \leq t \leq 1400 \\
0.2 & \mathrm{\ for\ } 1500 \leq t \leq 2000 \\
0 & \mathrm{\ otherwise}
\end{array}
\right.
\end{equation}

!listing modules/porous_flow/test/tests/dirackernels/squarepulse1.i

MOOSE produces the expected result:

!media dirackernels/squarepulse1.png style=width:50%;margin-left:10px caption=Results of a "square-pulsed" extraction and injection of fluid from a fluid-flow simulation.  id=squarepulse1_fig

## Theis tests

In the fully-saturated, isothermal case with no mechanical coupling and constant, large fluid bulk modulus, the [fluid flow equations](governing_equations.md) reduce to the form conventionally used in groundwater flow:
\begin{equation}
S_{s}\frac{\partial H}{\partial t} = \nabla_{i}\left(K_{ij}\nabla_{j} H \right) \ ,
\end{equation}
where

- $H$ is the "head", $H=(P - P_{0})/(\rho g) - \mathrm{depth}$.  Here $\rho$ is the fluid density.

- $S_{s}$ is the "specific storage".  If the rock compressibility is ignored then $S_{s} = \rho g \phi / K_{\mathrm{f}}$, where $K_{\mathrm{f}}$ is the fluid bulk modulus

- $K_{ij}$ is the hydraulic conductivity: $K_{ij} = k_{ij}\rho g / \mu$, where $k_{ij}$ is the permeability and $\mu$ is the fluid viscosity

This equation is identical to the physics described by the [PorousFlowFullySaturatedMassTimeDerivative](PorousFlowFullySaturatedMassTimeDerivative.md) and the [PorousFlowFullySaturatedDarcyBase](PorousFlowFullySaturatedDarcyBase.md) Kernels, which may be used explicitly, or through the [PorousFlowBasicTHM](PorousFlowBasicTHM.md) Action.  When using these, the specific storage $S_{s}$ in the above equation is replaced by $1/M$ (reciprocal of the Biot modulus), and $K_{ij}$ by $k_{ij}/\mu$, and $H$ by $P$.  Finally, remember that this formulation is *volume based*, not mass based like the remainder of PorousFlow.

Place a constant volumetric sink of strength $q$ (m$^{3}$.m$^{-1}$.s$^{-1}$) acting along an infinite line in an isotropic 3D medium.  The situation is therefore two dimensional.  Theis provided the solution for the head
\begin{equation}
H = H_{0} + \frac{q}{4\pi K}\mathrm{Ei}\left( -\frac{r^{2}S_{s}}{4Kt} \right) \ ,
\label{theis_soln_eqn}
\end{equation}
which is frequently used in the groundwater literature.  "Ei" is the exponential integral function, with values $\mathrm{Ei}(-x) = \gamma + log(x) - x + O(x^{2})$ for small $x$ (where $\gamma$ is the Euler number $0.57722\ldots$), and $\mathrm{Ei}(-x) = e^{-x}\left(-\frac{1}{x} + \frac{1}{x^{2}} + O(\frac{1}{x^{3}})\right)$ for large $x$.

This is checked using a number of PorousFlow tests (the extraction specified using mass rate, with volume rate, and from single and 2-phase systems).  For instance:

!listing modules/porous_flow/test/tests/dirackernels/theis_rz.i

MOOSE agrees with the Theis solution:

!media dirackernels/theis.png style=width:50%;margin-left:10px caption=Results of the fully-saturated, single-phase Theis simulation. id=theis_fig

Flow from a source in a radial 1D problem admits a similarity solution, where the fluid pressure (and saturation, mass fraction, etc) is a function of the similarity variable $\zeta = r^2/t$.  This may be observed in [theis_soln_eqn].  Using this fact, two-phase immiscible flow may be tested.  The following input file describes injection of a gas phase into a fully liquid-saturated model at a constant rate:

!listing modules/porous_flow/test/tests/dirackernels/theis3.i

[fig:theis_similarity_fig] shows the comparison of
similarity solutions calculated with either fixed radial distance or fixed time. In this case, good agreement
is observed between the two results for both liquid pressure and gas saturation. 

!media dirackernels/theis3_similarity_fig.png style=width:90%;margin-left:10px caption=Results of the 2-phase radial injection simulation. id=fig:theis_similarity_fig

Further tests of this kind, involving multi-component, multi-phase flow, including mutual dissolution of gas and liquid phases may be found [here](porous_flow/tests/fluidstate/fluidstate_tests.md).

## Peaceman borehole fluxes

The test suite that checks that the Peaceman flux
\begin{equation}
f(P_{i}, x_{i}) =
W \left|C(P_{i}-P_{\mathrm{bh}})\right|
\frac{k_{\mathrm{r}}\rho}{\mu}(P_{i} - P_{\mathrm{bh}})
\label{eq:peaceman_f}
\end{equation}
is correctly implemented.  A vertical borehole is placed through the
centre of a single element, and fluid flow to the borehole as a
function of porepressure is measured.  The tests are

- A production borehole with $P_{\mathrm{bh}} = 0$, with a fully-saturated medium.

!listing modules/porous_flow/test/tests/dirackernels/bh02.i

- An injection borehole with $P_{\mathrm{bh}} = 10\,$MPa, with a fully-saturated medium.

!listing modules/porous_flow/test/tests/dirackernels/bh03.i

- A production borehole with $P_{\mathrm{bh}} = -10\,$MPa with a fully-saturated medium, with `use_mobility=true`

!listing modules/porous_flow/test/tests/dirackernels/bh04.i

- An injection borehole with $P_{\mathrm{bh}} = 0$ with an unsaturated medium, with `use_mobility=true`

!listing modules/porous_flow/test/tests/dirackernels/bh05.i

Further commentary, and the results demonstrating that MOOSE is correct may be found in the page discussing the [theory behind Dirac Kernels in PorousFlow](sinks.md).

## Comparison with a steady-state 2D analytic solution

The test

!listing modules/porous_flow/test/tests/dirackernels/bh07.i

checks whether the Peaceman borehole extracts fluid at the correct rate and the corresponding reduction in porepressure agrees with an analytic solution of Laplace's equation.  Details and results demonstrating the correctness of PorousFlow can be found in the page discussing the [theory behind Dirac Kernels in PorousFlow](sinks.md).

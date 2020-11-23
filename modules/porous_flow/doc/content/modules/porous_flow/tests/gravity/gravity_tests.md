# Establishment of gravitational head in 1D

These tests concern the steady-state pressure distribution obtained
either by running a transient model for a long time, or by running a
steady-state analysis, both of which should lead to the same result.

Without fluxes, the steadystate pressure distribution is just
\begin{equation}
P(x) = P_{0} - \rho_{0} g x \ ,
\end{equation}
if the fluid bulk modulus, $B$, is large enough compared with $P$.
Here $P_{0}$ is the porepressure at $x=0$.  For smaller bulk modulus
\begin{equation}
P(x) = -B \log\left( e^{-P_{0}/B} + \frac{g\rho_{0}x}{B} \right) \ .
\label{grav.head.eqn}
\end{equation}
Here it is assumed that the density is given by $\rho = \rho_{0}e^{-P/B}$
with constant bulk modulus, $g$ is the
magnitude acceleration due to gravity (a vector assumed to be pointing in the
negative $x$ direction), and $x$ is position.  The tests described below
are simple tests and are part of the automatic test suite.

## Single-phase, single-component

Two single-phase simulations with 100 1D elements are run: one with
fully-saturated conditions, and the other with unsaturated conditions
using the van Genuchten capillary pressure (this should not, and does
not, make any difference to the results).  The porepressure is held
fixed at one boundary ($x=0$).  The parameters are tabulated in [tab:grav_params].

!table id=tab:grav_params caption=Parameter values used in the 1-phase tests
| Parameter | Value |
| --- | --- |
| x | $-1\leq x \leq 0\,$m |
| Bulk modulus (B) | 1.2 Pa |
| Reference density $\rho_{0}$ | 1 kg.m$^{-3}$ |
| gravitational acceleration $g$ | -1 m.s$^{-2}$ |

The steady-state input file with full saturation:

!listing modules/porous_flow/test/tests/gravity/grav01a.i

The steady-state input file with partial saturation

!listing modules/porous_flow/test/tests/gravity/grav01c.i

An example verification is shown in [gh.fig], which also
shows results from a 2-phase simulation (see next section)

!media gravity/gravity_fig.png style=width:90%;margin-left:10px caption=Comparison between the MOOSE result (in dots), and the
  exact analytic expression given by [grav.head.eqn]. id=gh.fig


## Two-phase, two-component

Two-phase, two-component simulations may also be checked against
[grav.head.eqn].  A number of simulations are performed with parameters tabulated in [tab:grav_params2].

!table id=tab:grav_params2 caption=Parameter values used in the 2-phase tests
| Parameter | Value |
| --- | --- |
| x | $-1\leq x \leq 0\,$m |
| Bulk modulus of heavy phase | 1.2 Pa |
| Bulk modulus of light phase | 1 Pa |
| Reference density of heavy phase | 1 kg.m$^{-3}$ |
| Reference density of light phase | 0.1 kg.m$^{-3}$ |
| gravitational acceleration $g$ | -1 m.s$^{-2}$ |

One steady-state simulation is performed.  Steady-state simulations are more
difficult to perform in two-phase situations because of the inherently
stronger nonlinearities, but mostly because simulations can easily enter
unphysical domains (negative saturation, for instance) without the stabilising
presence of the mass time-derivative.  The steady-state input file:

!listing modules/porous_flow/test/tests/gravity/grav02b.i

A variety of transient simulations are performed.  In the
transient simulations, conservation of mass can be checked, and the
tests demonstrate MOOSE conserves mass.  Depending on the initial and
boundary conditions, the "heavy" phase (with greatest mass) can
completely displace the "light" phase, which is forced to move to
the top of the simulation.  [grav.head.eqn] only governs the light phase in the unsaturated zone, since in the
saturated zone (where there is zero light phase) the pressure must
follow the heavy-phase version of [grav.head.eqn].  An
example is shown in [gh.fig].

Using the PP formulation with unsaturated fluids:

!listing modules/porous_flow/test/tests/gravity/grav02a.i

Using the PP formulation with saturated and unsaturated fluids::

!listing modules/porous_flow/test/tests/gravity/grav02c.i

Using the PP formulation with a boundary condition fixing porepressures:

!listing modules/porous_flow/test/tests/gravity/grav02d.i

Using the PS formulation with no residual saturation

!listing modules/porous_flow/test/tests/gravity/grav02e.i

Using the PS formulation with residual saturation

!listing modules/porous_flow/test/tests/gravity/grav02f.i

Using the PS formulation with Brooks-Corey capillarity and residual saturation

!listing modules/porous_flow/test/tests/gravity/grav02f.i







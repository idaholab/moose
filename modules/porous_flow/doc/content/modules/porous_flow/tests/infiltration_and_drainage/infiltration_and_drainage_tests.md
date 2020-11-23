# Infiltration and drainage test descriptions

## The 2-phase analytic infiltration solution

The physical setup studied in this section is a 1D column that is
initially unsaturated, and which is subject to a constant injection of
fluid from its top.  This is of physical importance because it is a
model of constant rainfall recharge to an initially dry groundwater system.
The top surface becomes saturated, and this saturated zone moves
downwards into the column, diffusing as it goes.  The problem is of
computational interest because under certain conditions an analytic
solution is available for the saturation profile as a function of
depth and time.

The Richards' equation for an incompressible fluid in one
spatial dimension ($z$) reads
\begin{equation}
\dot{S} = \nabla \left(D \nabla S\right) - \nabla K \ ,
\end{equation}
where
\begin{equation}
D(S) = -\frac{\kappa \kappa_{rel}}{\mu\phi}P_{\mathrm{c}}' \ ,
\end{equation}
and
\begin{equation}
K(S) = \frac{\rho g \kappa\kappa_{\mathrm{rel}}}{\mu\phi} \ .
\end{equation}
Here $P_{\mathrm{c}} = -P$ which is the capillary pressure, and recall
that $P_{\mathrm{c}}'(S)<0$.

The analytic solution of this nonlinear diffusion-advection relevant
to constant infiltration to groundwater has been derived by Broadbridge
and White [!citep](broadbridge1988) for certain functions
$D$ and $K$.   Broadbridge and White
assume the hydraulic conductivity is
\begin{equation}
K(S) = K_{n} + (K_{s}-K_{n})\frac{\Theta^{2}(C-1)}{C-\Theta} \ ,
\label{eq:bw_krel}
\end{equation}
where
\begin{equation}
\Theta = \frac{S - S_{n}}{S_{s} - S_{s}} \ ,
\end{equation}
and the parameters obey $0 \leq K_{n} < K_{s}$, $0 \leq S_{n} \leq S
\leq S_{s}\leq 1$, and $C>1$.  The diffusivity is of the form
$a(b-S)^{-2}$.  This leads to very complicated relationships between
the capillary pressure, $P_{c}$, and the saturation, except in the
case where $K_{n}$ is small, when they are related through
\begin{equation}
\frac{P_{\mathrm{c}}}{\lambda_{s}} = \frac{\Theta - 1}{\Theta} - \frac{1}{C}\log
\left( \frac{C-\Theta}{(C-1)\Theta} \right) \ ,
\end{equation}
with $\lambda_{s}>0$ being the final parameter introduced by
Broadbridge and White.

Broadbridge and White derive time-dependent solutions for constant
recharge to one end of a semi-infinite line.  Their solutions are
quite lengthy, will not be written here.  To compare with MOOSE,
the following parameters are used --- the hydraulic parameters are those
used in Figure 3 of Broadbridge and White:

!table id=tab:bw_params caption=Parameter values used in the 2-phase tests
| Parameter | Value |
| --- | --- |
| Bar length | 20$\,$m |
| Bar porosity | 0.25 |
| Bar permeability | 1 |
| Gravity | 0.1$\,$m.s$^{-2}$ |
| Fluid density | 10$\,$kg.m$^{-3}$ |
| Fluid viscosity | 4$\,$Pa.s |
| $S_{n}$ | 0$\,$m.s$^{-1}$ |
| $S_{s}$ | 1$\,$m.s$^{-1}$ |
| $K_{n}$ | 0$\,$m.s$^{-1}$ |
| $K_{s}$ | 1$\,$m.s$^{-1}$ |
| $C$ | 1.5 |
| $\lambda_{s}$ | 2$\,$Pa |
| Recharge rate $R_{\ast}$ | 0.5 |

Broadbridge and white consider the case where the initial condition is
$S=S_{s}$, but this yields $P=-\infty$, which is impossible to use in
a MOOSE model.  Therefore the initial condition $P=-900\,$Pa is used
which avoids any underflow problems.  The recharge rate of
$R_{\ast}=0.5$ corresponds in the MOOSE model to a recharge rate of
$0.5\rho\phi(\kappa_{s}-\kappa_{n})=1.25\,$kg.m$^{-2}$.s$^{-1}$.  Note
that $\frac{\rho g \kappa}{\mu \phi} = 1\,$m.s$^{-1}$, so
that the $K_{n}$ and $K_{s}$ may be encoded as $\kappa_{n}=0$ and
$\kappa_{s}=1$ in the relative permeability function
[eq:bw_krel] in a straightforward way.

[bw_fig] shows good agreement between the analytic solution
of Broadbridge and White and the MOOSE implementation.  There are
minor discrepancies for small values of saturation: these get smaller
as the temporal and spatial resolution is increased, but never totally
disappear due to the initial condition of $P=-900\,$Pa.


!media infiltration_and_drainage/bw.png style=width:50%;margin-left:10px caption=Comparison of the Broadbridge and White analytical solution with the MOOSE solution for 3 times.  This figure is shown in the standard format used in the Broadbridge-White paper: the constant recharge is applied to the top (where depth is zero) and gravity acts downwards in this figure.  id=bw_fig

Two tests are part of the automatic test suite (one is marked
"heavy" because it is a high-resolution version).

!listing modules/porous_flow/test/tests/infiltration_and_drainage/bw01.i

## The two-phase analytic drainage solution

Warrick, Lomen and Islas [!citep](warrick1990) extended
the analysis of Broadbridge and White to include
the case of drainage from a medium.

The setup is an initially-saturated semi-infinite column of material
that drains freely from its lower end.  This is simulated by placing a
boundary condition of $P=0$ at the lower end.  To obtain their analytical
solutions, Warrick, Lomen and Islas make the same assumptions as
Broadbridge and White concerning the diffusivity and conductivity of
the medium.  Their solutions are quite lengthy, so are not written here

A MOOSE model with the parameters almost identical to those listed in
[tab:bw_params] is compared with the analytical solutions.  The only
differences are that the "bar" length is $10000\,$m (to avoid any
interference from the lower Dirichlet boundary condition), and
$R_{\ast}=0$ since there is no recharge.  The initial condition is
$P=10^{-4}\,$Pa: the choice $P=0$ leads to poor convergence since
by construction the Broadbridge-White capillary function is only
designed to simulate the unsaturated zone $P<0$ and a sensible
extension to $P\geq 0$ is discontinuous at $P=0$.

[wli_fig] shows good agreement between the analytic
solution and the MOOSE implementation.  Any minor discrepancies get
smaller as the temporal and spatial resolution increase.

!media infiltration_and_drainage/wli.png style=width:50%;margin-left:10px caption=Comparison of the Warrick, Lomen and Islas analytical solution with the MOOSE solution for 3 times.  This figure is shown in the standard format used in the literature: the top of the model is at the top of the figure, and gravity acts downwards in this figure, with fluid draining from the infinitely deep point.  id=wli_fig

Two tests are part of the automatic test suite (one is marked
"heavy" because it is a high-resolution version).

!listing modules/porous_flow/test/tests/infiltration_and_drainage/wli01.i

## Single-phase infiltration and drainage

Forsyth, Wu and Pruess [!citep](forsyth1995) describe a HYDRUS simulation of an
experiment involving infiltration (experiment 1) and subsequent
drainage (experiment 2) in a large caisson.  The simulation is
effectively one dimensional, and is shown in
[rd_setup_fig].

!media infiltration_and_drainage/rd_setup.png style=width:50%;margin-left:10px caption=Two experimental setups from Forsyth, Wu and Pruess.  Experiment 1 involves infiltration of water into an initially unsaturated caisson.  Experiment 2 involves drainage of water from an initially saturated caisson.  id=rd_setup_fig

The properties common to each experiment
are listed in [tab:caisson_params]

!table id=tab:caisson_params caption=Parameter values used in the single-phase infiltration and drainage tests
| Parameter | Value |
| --- | --- |
Caisson porosity | 0.33 |
Caisson permeability | $2.95\times 10^{-13}\,$m$^{2}$ |
Gravity | 10$\,$m.s$^{-2}$ |
Water density at STP | 1000$\,$kg.m$^{-3}$ |
Water viscosity | 0.00101$\,$Pa.s |
Water bulk modulus | 20$\,$MPa |
Water residual saturation | 0.0 |
Air residual saturation | 0.0 |
Air pressure | 0.0 |
van Genuchten $\alpha$ | $1.43\times 10^{-4}\,$Pa$^{-1}$ |
van Genuchten $m$ | 0.336 |
van Genuchten turnover | 0.99 |

In each experiment 120 finite elements are used along the length of
the Caisson.  The modified van Genuchten relative permeability curve
with a "turnover" (set at $S=0.99$) is employed in order to improve
convergence significantly.  Hydrus also uses a modified van Genuchten
curve, although I couldn't find any details on the modification.

In experiment 1, the caisson is initially at saturation 0.303
($P=-72620.4\,$Pa), and water is pumped into the top with a rate
0.002315$\,$kg.m$^{-2}$.s$^{-1}$.  This causes a front of water to
advance down the caisson.  [rd01_result_fig] shows the
agreement between MOOSE and the published result (this result was
obtained by extracting data by hand from online graphics).

In experiment 2, the caisson is initially fully saturated at $P=0$,
and the bottom is held at $P=0$ to cause water to drain via the action
of gravity.  [rd01_result_fig] and [rd02_result_fig] show the agreement between
MOOSE and the published result.

!media infiltration_and_drainage/rd01.png style=width:50%;margin-left:10px caption=Saturation profile in the caisson after 4.16 days of infiltration.  Note that the HYDRUS results are only approximate they were extrated by hand from online graphics.  id=rd01_result_fig

!media infiltration_and_drainage/rd02.png style=width:50%;margin-left:10px caption=Saturation profiles in the caisson after drainage from an initially-saturated simulation (4 days and 100 days profiles).  Note that the HYDRUS results are only approximate they were extrated by hand from online graphics.  id=rd02_result_fig

Experiment 1 and the first 4 simulation days of experiment 2 are
marked as "heavy" in the PorousFlow test suite since the simulations
take around 3 seconds to complete.

The input file for Experiment 1:

!listing modules/porous_flow/test/tests/infiltration_and_drainage/rd01.i

The input file for the first 4 simulation days of Experiment 2:

!listing modules/porous_flow/test/tests/infiltration_and_drainage/rd02.i

The input file for the latter 96 days of Experiment 2:

!listing modules/porous_flow/test/tests/infiltration_and_drainage/rd03.i


## Water infiltration into a two-phase (oil-water) system

An analytic solution of the two-phase Richards' equations with
gravity on a
semi-infinite line $z\geq 0$, with a constant water infiltration flux
at $z=0$ has been derived by [!citep](rsc1983) (Unfortunately there must be a typo in the RSC paper
  as for nonzero gravity their results are clearly incorrect.).  The authors
assume incompressible fluids; linear relative permeability
relationships; the "oil" (or "gas") viscosity is larger than the
water viscosity; and, a certain functional form for the capillary
pressure.  When the oil viscosity is exactly twice the water
viscosity, their effective saturation reads
\begin{equation}
S_{\mathrm{eff}} = \frac{1}{\sqrt{1 + \exp((P_{c} - A)/B)}} \ ,
\label{eqn.rsc.seff}
\end{equation}
where $P_{c} = P_{\mathrm{oil}}-P_{\mathrm{water}}$ is the capillary
pressure, and $A$ and $B$ are arbitrary parameters to be defined by
the user in the PorousFlow implementation.  For other oil/water viscosity
ratios $P_{c} = P_{c}(S_{\mathrm{eff}})$ is more complicated, and note
that their formulation allows $P_{c}<0$, but only
the particular form [eqn.rsc.seff] need be used to validate
the MOOSE implementation.

RSC's solutions are quite lengthy, so I will not write them here.  To
compare with MOOSE, the following parameters are used:

!table id=rsc_params caption=Parameter values used in the Rogers-Stallybrass-Clements infiltration tests
| Parameter | Value |
| --- | --- |
| Bar length | 10 m |
| Bar porosity | 0.25 |
| Bar permeability | $10^{-5}\,$m$^{2}$ |
| Gravity | 0 m.s$^{-2}$ |
| Water density | 10 kg.m$^{-3}$ |
| Water viscosity | $10^{-3}\,$Pa.s |
| Oil density | 20 kg.m$^{-3}$ |
| Oil viscosity | $2\times 10^{-3}\,$Pa.s |
| Capillary $A$ | 10 Pa |
| Capillary $B$ | 1 Pa |
| Initial water pressure | 0 Pa |
| Initial oil pressure | 15 Pa |
| Initial water saturation | 0.08181 |
| Initial oil saturation | 0.91819 |
| Water injection rate | 1 kg.s$^{-1}$.m$^{-2}$ |

The input file:

!listing modules/porous_flow/test/tests/infiltration_and_drainage/rsc01.i

In the RSC theory water is injected into a semi-infinite domain,
whereas of course the MOOSE implementation has finite extent ($0\leq z
\leq 10$ is chosen).  Because of the near incompressibility of the
fluids (I choose the bulk modulus to be 2 GPa) this causes the
porepressures to rise enormously, and the problem can suffer from
precision-loss problems.  Therefore, the porepressures are fixed at
$z=10$.  This does not affect the progress of the water saturation
front.

[rsc.fig] shows good agreement between the analytic
solution and the MOOSE implementation.  Any minor discrepancies get
smaller as the temporal and spatial resolution increase, as is
suggested by the two comparisons in that figure.

The "low-resolution" test has 200 elements in $0\leq z\leq
10$ and uses 15 time steps is part the automatic test suite that
is run every time the code is updated.  The "high-resolution" test
has 600 elements and uses 190 time steps, and is marked as
"heavy".

!media infiltration_and_drainage/rsc.png style=width:50%;margin-left:10px caption=Water saturation profile after 5 seconds of injection in the Rogers-Stallybrass-Clements test.  The initial water saturation is 0.08181, and water is injected at the top of this figure at a constant rate.  This forms a water front which displaces the oil.  Black line: RSC's analytic solution.  Red squares: high-resolution MOOSE simulation.  Green triangles: lower resolution MOOSE simulation.  id=rsc.fig

!bibtex bibliography

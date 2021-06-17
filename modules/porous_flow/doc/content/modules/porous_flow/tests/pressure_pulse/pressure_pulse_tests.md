# Pressure-pulses in 1D

The PorousFlow fluid equation for single-phase flow through a fully saturated medium without
gravity and without sources is just Darcy's equation
\begin{equation}
\frac{\partial}{\partial t}\phi\rho = \nabla_{i}\left(\frac{\rho
  \kappa_{ij}}{\mu} \nabla_{j}P \right) \ ,
\end{equation}
with notation described in [the governing equations](governing_equations.md).  Using $\rho \propto
\exp(P/K)$, where $K$ is the fluid bulk modulus, Darcy's equation
becomes
\begin{equation}
\frac{\partial}{\partial t}\rho = \nabla_{i}\alpha_{ij}\nabla\rho \ ,
\end{equation}
with
\begin{equation}
\alpha_{ij} = \frac{\kappa_{ij}B}{\mu\phi} \ .
\end{equation}
Here I've assumed the porosity and bulk modulus are constant in space
and time.

Consider the one-dimensional case were the spatial dimension is the
semi-infinite line $x\geq 0$.  Suppose that initially the pressure is
constant, so that
\begin{equation}
\rho(x, t=0) = \rho_{0} \ \ \ \mathrm{for }\ \ x\geq 0 \ .
\end{equation}
Then apply a fixed-pressure Dirichlet boundary condition at $x=0$ so
that
\begin{equation}
\rho(x=0, t>0) = \rho_{\infty}
\end{equation}
The solution of the above differential equation is well known to be
\begin{equation}
\rho(x, t) = \rho_{\infty} + (\rho_{0} -
\rho_{\infty})\,\mathrm{Erf}\left( \frac{x}{\sqrt{4\alpha t}} \right) \ ,
\label{eqn.exact.pp}
\end{equation}
where Erf is the error function.

A number of PorousFlow tests verify that this solution is produced with parameters shown in [tab:pp]

!table id=tab:pp caption=Parameter values used in the pressure-pulse tests
| Parameter | Value |
| --- | --- |
| length of 1D bar | 100 m |
| number of elements | 10 |
| end time (for transient simulations) | 10000 s |
| number of time steps | 10 |
| Fluid bulk modulus ($B$) | 2 GPa |
| Fluid viscosity ($\mu$) | 0.001 Pa.s |
| Permeability ($\kappa_{xx}$) | $10^{-15}\,$m$^{2}$ |
| Porosity | 0.1 |
| Initial pressure | 2 MPa |
| Applied pressure | 3 MPa |

The tests include:

- Steady state 1-phase analysis to demonstrate that the steady-state of $\rho = \rho_{\infty}$ is achieved.

!listing modules/porous_flow/test/tests/pressure_pulse/pressure_pulse_1d_steady_action.i

- Transient 1-phase analysis.

!listing modules/porous_flow/test/tests/pressure_pulse/pressure_pulse_1d_action.i

- Transient 1-phase, 3 component analysis to check that the components diffuse at the same rate.

!listing modules/porous_flow/test/tests/pressure_pulse/pressure_pulse_1d_3comp_action.i

- Transient 2-phase analysis, with the "water" state fully saturated.

!listing modules/porous_flow/test/tests/pressure_pulse/pressure_pulse_1d_2phase.i

- Transient 2-phase using the "PS" formulation

- 2-phase using the "PS" formulation and [Kuzmin-Turek](kt.md) stabilisation

- 2 phase using the PS formulation and a van Genuchten capillary pressure

- 2 phase using the PS formulation and a van Genuchten capillary pressure with a logarithmic extension;

- 1 phase when the primary variable is $\mathrm{log}\,\rho$

- 1 phase using the [fully-saturated](PorousFlowFullySaturatedDarcyFlow.md) [version](PorousFlowFullySaturatedMassTimeDerivative.md) of PorousFlow (no upwinding and no mass lumping)

- 1 phase with 3 fluid components using the [fully-saturated](PorousFlowFullySaturatedDarcyFlow.md) [version](PorousFlowFullySaturatedMassTimeDerivative.md) of PorousFlow (no upwinding and no mass lumping)

An example verification is shown in [pressure_pulse.fig].

!media pressure_pulse/pressure_pulse_1d.png style=width:80%;margin-left:10px caption=Comparison between the MOOSE result (in dots), and the exact analytic expression given by [eqn.exact.pp].  The agreement increases for greater spatial resolution and smaller timesteps.  Both the multi-component single-phase simulation (using the fully-saturated non-upwinding Kernels, or the partially-saturated full-upwinding Kernels, or the log(mass-density) primary variable) and the 2-phase fully-water-saturated simulation give identical results for the water porepressure. id=pressure_pulse.fig

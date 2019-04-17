# Heat conduction tests descriptions

## Simplifying PorousFlow's heat equation

PorousFlow heat conduction is governed by the equation
\begin{equation}
0 = \frac{\partial}{\partial t}{\mathcal{E}} + \nabla\cdot {\mathbf{F}}^{\mathrm{T}} \ ,
\label{eqn.heat.cond}
\end{equation}
Here ${\mathcal{E}}$ is the energy per unit volume in the rock-fluid
system, and ${\mathbf{F}}^{\mathrm{T}}$ is the heat flux.  In the
PorousFlow module
\begin{equation}
{\mathcal{E}} = (1 - \phi)\rho_{R}C_{R} T + \phi
\sum_{{\beta}}S_{{\beta}}\rho_{{\beta}}{\mathcal{E}}_{{\beta}} \ ,
\end{equation}
when there is no adsorbed species.  Here $T$ is the temperature,
$\phi$ is the rock porosity, and $S_{{\beta}}$ is the saturation of
phase ${\beta}$.  The remainder of the notation is described in the
next paragraph.  When studying problems involving heat conduction
(with no fluid convection)
\begin{equation}
{\mathbf{F}}^{\mathrm{T}} = -\lambda\nabla T \ ,
\end{equation}
where $\lambda$ is the tensorial thermal conductivity of the
rock-fluid system.

The tests described in this page use the following simple forms for
each term

- $\rho_{R}$, which is the rock-grain density (that is, the density of rock with zero porespace), measured in kg.m$^{-3}$, is assumed constant in the current implementation of the PorousFlow module.

- $C_{R}$, which is the rock-grain specific heat capacity, measured in J.kg$^{-1}$.K$^{-1}$, is assumed constant in the current implementation of the PorousFlow module.

- $\rho_{{\beta}}$, which is the density of fluid phase ${\beta}$, is assumed in here to be a function of the fluid pressure only.  This is so [eqn.heat.cond] may be easily solved, but more general forms are allowed in the PorousFlow module.

- ${\mathcal{E}}_{{\beta}}$, which is the specific internal energy of the fluid phase ${\beta}$, and is measured in J.kg$^{-1}$, is assumed here to be ${\mathcal{E}}_{{\beta}} = C_{v}^{{\beta}} T$, where $C_{v}^{{\beta}}$ is the fluid's specific heat capacity at constant volume.  This specific heat capacity is assumed constant, so that [eqn.heat.cond] may be easily solved --- more general forms are allowed in the PorousFlow module.

- $\lambda$ is assumed to vary between $\lambda^{\mathrm{dry}}$ and $\lambda^{\mathrm{wet}}$, depending on the aqueous saturation: $\lambda_{ij} = \lambda_{ij}^{\mathrm{dry}} + S^{n} \left(\lambda_{ij}^{\mathrm{wet}} - \lambda_{ij}^{\mathrm{dry}} \right)$, where $S$ is the aqueous saturation, and $n$ is a positive user-defined exponent.  More general forms may be easily accommodated in the PorousFlow module, but to date none have been coded.

Under these conditions, [eqn.heat.cond] becomes
\begin{equation}
\dot{T} = \nabla_{i} \alpha_{ij} \nabla_{j} T \ .
\label{eqn.heat.cond.simple}
\end{equation}
The tensor $\alpha$ is
\begin{equation}
\alpha_{ij} = \frac{\lambda_{ij}}{(1 - \phi)\rho_{R}C_{R} +
  \rho\sum_{{\beta}}S_{{\beta}}\rho_{{\beta}}C_{v}^{{\beta}}} \ .
\end{equation}
For constant saturation and porepressure, $\alpha_{ij}$ is also constant.

## Testing heat conduction in 1D

Consider the one-dimensional case where the spatial dimension is the
semi-infinite line $x\geq 0$.  Suppose that initially the temperature is
constant, so that
\begin{equation}
T(x, t=0) = T_{0} \ \ \ \mathrm{for }\ \ x\geq 0 \ .
\end{equation}
Then apply a fixed-pressure Dirichlet boundary condition at $x=0$ so
that
\begin{equation}
T(x=0, t>0) = T_{\infty}
\end{equation}
The solution of the above differential equation is well known to be
\begin{equation}
T(x, t) = T_{\infty} + (T_{0} -
T_{\infty})\,\mathrm{Erf}\left( \frac{x}{\sqrt{4\alpha t}} \right) \ ,
\label{eqn.exact.pp}
\end{equation}
where Erf is the error function.

This is verified by using the following tests on a line of 10 elements.

- A transient analysis with no fluids.  The parameters chosen are $\lambda_{ij} = \mathrm{diag}\,(2.2)$, $\phi=0.9$, $\rho_{R}=0.5$ and $C_{R}=2.2$ is chosen, so that $\alpha_{ij} = 1/(0.9\times 0.5)$.

!listing modules/porous_flow/test/tests/heat_conduction/no_fluid.i

- A transient analysis with 2 fluid phases.  The parameters chosen are $\lambda^{\mathrm{dry}}=0.3$, $\lambda^{\mathrm{wet}} = 1.7$ and $S=0.5$, so that $\lambda_{ij} = \mathrm{diag}\,(1)$.  $\rho_{\mathrm{gas}} = 0.4$, $\rho_{\mathrm{water}} = 0.3$, $\rho_{R}=0.25$, $C_{v}^{\mathrm{gas}} = 1$, $C_{v}^{\mathrm{water}}=2$, $C_{R}=1.0$, and $\phi=0.8$.  With these parameters, $\alpha_{ij} = 1/(0.9\times 0.5)$.

!listing modules/porous_flow/test/tests/heat_conduction/two_phase.i

PorousFlow yields the expected answer, as shown in [heat_conduction.fig].

!media heat_conduction/heat_conduction_1d.png style=width:50%;margin-left:10px caption=Comparison between the MOOSE result and the exact analytic expression given by [eqn.exact.pp].  id=heat_conduction.fig

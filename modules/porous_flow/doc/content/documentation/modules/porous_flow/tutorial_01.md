[Start](tutorial_00.md) |
[Previous](tutorial_00.md) |
[Next](tutorial_02.md)

# Porous Flow Tutorial Page 01.  A single fluid

This tutorial page describes how PorousFlow can be used to solve a
very simple fluid-flow problem.  The physics are described by the
equation
\begin{equation}
\label{eq:basicthm}
0 = \frac{\dot{P}}{M} + \alpha_{B}\dot{\epsilon}_{v} - A\dot{T} -
\nabla_{i}\left(\frac{k_{ij}}{\mu}\left(\nabla_{j}P - \rho g_{j}\right)\right)
\ .
\end{equation}
This is the simplest type of equation that PorousFlow solves.  It is often used in thermo-poro-elasticity, but PorousFlow is often employed to solve much more complex equations.  See [governing equations](/porous_flow/governing_equations.md) for further details, and [PorousFlowFullySaturatedMassTimeDerivative](PorousFlowFullySaturatedMassTimeDerivative.md) and [PorousFlowFullySaturatedDarcyBase](PorousFlowFullySaturatedDarcyBase.md) for the derivation of [eq:basicthm] from the general governing equations.  In this equation:

- $P$ is the fluid porepressure (units of Pa)
- an over-dot represents a time derivative
- $M$ is the Biot Modulus (units of Pa)
- $\alpha_{B}$ is the Biot coefficient (dimensionless)
- $\dot{\epsilon}_{v}$ is the rate of volumetric strain of the solid rock (units s$^{-1}$)
- $A$ is the effective volumetric thermal expansion coefficient (units K$^{-1}$)
- $T$ is the temperature (units K)
- $\nabla$ represents a spatial derivative
- $k_{ij}$ is the permeability tensor (units m$^{2}$)
- $\mu$ is the fluid viscosity (units Pa.s)
- $\rho$ is the fluid density (units kg.m$^{-3}$)
- $g_{j}$ is the gravitational acceleration (units m.s$^{-2}$)

The units suggested here are not manditory: any consistent unit system
may be used in MOOSE.  For instance, in reservoir physics it is often
convenient to express everything in MPa and years rather than Pa and
seconds.

The Biot modulus is
\begin{equation}
\frac{1}{M} = \frac{\phi}{K_{f}} + \frac{(1 - \alpha_{B})(\alpha_{B} - \phi)}{K} \ ,
\end{equation}
and the effective volumetric thermal expansion coefficient is
\begin{equation}
A = (\alpha_{B} - \phi)\alpha_{T} + \phi\alpha_{f} \ .
\end{equation}
In these equations

- $\phi$ is the porosity (dimensionless)
- $K_{f}$ is the bulk modulus of the fluid (units Pa$^{-1}$)
- $K$ is the bulk modulus of the drained porous skeleton (units Pa$^{-1}$)
- $\alpha_{T}$ is the volumetric thermal expansion coefficient of the drained porous skeleton (units K$^{-1}$)
- $\alpha_{f}$ is the volumetric thermal expansion coefficient of the fluid (units K$^{-1}$)

The [derivation](PorousFlowFullySaturatedMassTimeDerivative.md) of [eq:basicthm] from
the [full PorousFlow equations](governing_equations.md) assumes that $M$ and $A$ are constant.

In this tutorial page we will be solving fluid flow only, so the
$\dot{\epsilon}_{v}$ and $\dot{T}$ in Eq. [eq:basicthm] are ignored (set to zero).

*All* PorousFlow input files must contain a [`PorousFlowDictator`](PorousFlowDictator.md), and almost all PorousFlow objects (`Kernels`, `Materials`, etc) require the `PorousFlowDictator` to be provided.  This enables PorousFlow to make consistency checks on the number of fluid phases, components, chemical reactants, etc.  Therefore, most input files specify its name in the `Globals` block:

!listing modules/porous_flow/examples/tutorial/01.i start=[GlobalParams] end=[Variables]

Most PorousFlow simulations require fluid properties to be supplied.  In this instance, the [`SimpleFluidProperties`](SimpleFluidProperties.md) are used, which assume a constant fluid bulk modulus and viscosity:

!listing modules/porous_flow/examples/tutorial/01.i start=[Modules] end=[Materials]

The DE of [eq:basicthm] is implemented in the following way

!listing modules/porous_flow/examples/tutorial/01.i start=[Variables] end=[BCs]

There is just one variable --- the porepressure --- and there is no coupling with heat or mechanics.  Gravity is set to zero.  The [`PorousFlowBasicTHM`](PorousFlowBasicTHM.md) has other optional inputs that you are encouraged to explore, including setting the temperature to a non-default value, or to the value of an AuxVariable (your fluid properties may depend on temperature, even in an isothermal situation).

In this tutorial page, the only boundary condition is to fix the porepressure to 1$\,$MPa at the injection area (the other boundaries default to zero flux):

!listing modules/porous_flow/examples/tutorial/01.i start=[BCs] end=[Modules]

The porosity, Biot modulus and permeability are defined through the Materials block:

!listing modules/porous_flow/examples/tutorial/01.i start=[Materials] end=[Preconditioning]

The result is shown in [tut01_gif_fig]

!media porous_flow/tut01.gif style=width:50%;margin-left:10px caption=Porepressure evolution in the borehole-aquifer-caprock system.  id=tut01_gif_fig

[Start](tutorial_00.md) |
[Previous](tutorial_00.md) |
[Next](tutorial_02.md)

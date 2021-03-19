[Start](porous_flow/tutorial_00.md) |
[Previous](porous_flow/tutorial_02.md) |
[Next](porous_flow/tutorial_04.md)

# Porous Flow Tutorial Page 03.  Adding heat advection and conduction

This Page adds heat conduction and advection with the fluid.  The differential equation governing the temperature evolution is
\begin{equation}
\label{eq:heat}
0 = \frac{\partial}{\partial t}\left((1 - \phi)\rho_{R}C_{R}T + \phi\rho C_{v}T \right) - \nabla_{i}\lambda_{ij}\nabla T  -\nabla_{i} \left( C_{v} T \rho \frac{k_{ij}}{\mu} (\nabla_{j}P - \rho g_{j}) \right)\ .
\end{equation}

This equation is nonlinear since there are products of $P$ and $T$, as well as the nonlinear function $\rho$.  Most of the nomenclature was used in [Page 01](porous_flow/tutorial_01.md), and the additional symbols introduced are:

- $t$ is time (units s)

- $\rho_{R}$ is the density of the rock grains (units kg.m$^{-3}$)

- $C_{R}$ is the specific heat capacity of the rock grains (units
  J.kg$^{-1}$.K$^{-1}$)

- $T$ is the temperature (units K)

- $C_{v}$ is the specific heat capacity of the fluid (units
  J.kg$^{-1}$.K$^{-1}$)

- $\lambda_{ij}$ is the thermal conductivity of the rock-fluid system
  (units J.s$^{-1}$.m$^{-1}$.K$^{-1}$).  It is a tensor.

Before attempting to write an input file, a rough estimate of the expected nonlinear residuals must be performed, as discussed in [convergence criteria](porous_flow/convergence.md).  The residual for the [eq:heat] is approximately

\begin{equation}
R_{T} \approx V(\lambda \epsilon_{T} + C_{\mathrm{v}}T|\kappa|\rho_{0}\epsilon_{P}/\mu) \approx V (10 \epsilon_{T} + 10^{-2}\epsilon_{P}) \ ,
\end{equation}

where the parameters $\lambda \sim 10$, $C_{\mathrm{v}} \sim 4000$, $T\sim 300$, $\kappa \sim 10^{-14}$, $\rho_{0}\sim 1000$ and $\mu \sim 10^{-3}$, have been used in the final expression.  In [Page 02](porous_flow/tutorial_02.md) the choice $\epsilon_{P}\approx 1\,$Pa.m$^{-1}$ was made.  Choosing $\epsilon_{T}\approx 10^{-3}\,$K.m$^{-1}$ yields

\begin{equation}
R_{T} \approx 10^{-2}V \ .
\end{equation}

Note that this is significantly greater than the $R_{P}\approx 10^{-10}V$ for the fluid equation.  In the main, MOOSE can handle these types of discrepancies, but it is good practise to scale the variables so that their residuals are of similar magnitude.  Therefore, a scaling factor of $10^{-8}$ is applied to the temperature variable.

To model this thermo-hydro system, the `PorousFlowBasicTHM` action needs to be enhanced to read:

!listing modules/porous_flow/examples/tutorial/03.i start=[Variables] end=[BCs]

and some extra properties need to be added to the [`SimpleFluidProperties`](SimpleFluidProperties.md):

!listing modules/porous_flow/examples/tutorial/03.i start=[Modules] end=[Materials]

The boundary conditions used are the same as in [Page 01](porous_flow/tutorial_01.md) in addition to specifying a constant injection temperature of 313$\,$K:

!listing modules/porous_flow/examples/tutorial/03.i start=[constant_injection_temperature] end=[]

Finally, some temperature-related `Materials` need to be defined

!listing modules/porous_flow/examples/tutorial/03.i start=[thermal_expansion] end=[Preconditioning]

An animation of the results is shown in [tut03_gif_fig].  Readers are encouraged pause and explore the effect of changing parameters such as the rock thermal conductivity.

!media porous_flow/tut03.gif style=width:50%;margin-left:10px caption=Temperature evolution in the borehole-aquifer-caprock system.  id=tut03_gif_fig


[Start](porous_flow/tutorial_00.md) |
[Previous](porous_flow/tutorial_02.md) |
[Next](porous_flow/tutorial_04.md)

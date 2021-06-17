[Start](porous_flow/tutorial_00.md) |
[Previous](porous_flow/tutorial_03.md) |
[Next](porous_flow/tutorial_05.md)

# Porous Flow Tutorial Page 04.  Adding solid mechanics

In this Page, solid mechanics is added to the thermo-hydro simulation of preious Pages.  The equations are discussed in [governing equations](porous_flow/governing_equations.md).  Only quasi-static solid mechanics is considered here, without gravity, so the equations read
\begin{equation}
\sigma_{ij}^{\mathrm{eff}}  = \sigma_{ij}^{\mathrm{tot}} + \alpha_{B}\delta_{ij}P
\end{equation}
\begin{equation}
\sigma_{ij}^{\mathrm{eff}}  =  E_{ijkl}(\epsilon_{kl}^{\mathrm{elastic}} - \delta_{kl}\alpha^{\mathrm{linear}}_{T}(T - T_{\mathrm{ref}}))
\end{equation}
\begin{equation}
\label{eq:solid}
0  =  \nabla_{i}\sigma_{ij}^{\mathrm{eff}} - \alpha_{B}\nabla_{j}P \ .
\end{equation}
As described previously, $P$ is the porepressure, $T$ the temperature and $\alpha_{B}$ is the Biot coefficient.  The additional nomenclature used here is

- $\sigma^{\mathrm{eff}}$ is the effective stress tensor

- $\sigma^{\mathrm{tot}}$ is the total stress tensor

- $E_{ijkl}$ is the elasticity tensor of the drained porous skeleton

- $\alpha^{\mathrm{linear}}_{T}$ is the linear thermal expansion
  coefficient.  Note that this is the *linear* version, in contrast to
  the volumetric coefficients introduced in [Page 1](porous_flow/tutorial_01.md).

Once again, before  attempting to write an input file, a rough estimate of the expected nonlinear residuals must be performed, as discussed in [convergence criteria](porous_flow/convergence.md).  The residual for the [eq:solid] is approximately
\begin{equation}
R_{\sigma} \approx V\epsilon_{\sigma}
\end{equation}
Corresponding to the choice $\epsilon_{P}\approx  1\,$Pa.m$^{-1}$ made in [Page 02](porous_flow/tutorial_02.md) the choice $\epsilon_{\sigma}\approx 1\,$Pa.m$^{-1}$ may be made here.  This means $R_{\sigma}\approx V$ which is significantly greater than $R_{P}\approx 10^{-10}V$ for the fluid equation.  Therefore, the displacement variables are scaled by $10^{-10}$.

Many mechanically-related MOOSE objects (`Kernels`, `BCs`, etc) accept the `use_displaced_mesh` input parameter.  For virtually all PorousFlow simulations, it is appropriate to set this to false: `use_displaced_mesh = false`.  This means that the Kernel's residual (or BC's residual, Postprocessor's value, etc) will be evaluated using the undisplaced mesh.  This has the great numerical advantage that the solid-mechanics elasticity equations remain linear.

Also, many mechanically-related MOOSE objects require the `displacements` input parameter.  Therefore, it is convenient to put this parameter into the `GlobalParams` block:

!listing modules/porous_flow/examples/tutorial/04.i start=[GlobalParams] end=[Variables]

To model this thermo-hydro-mechanical system, the `PorousFlowBasicTHM` action needs to be enhanced to read:

!listing modules/porous_flow/examples/tutorial/04.i start=[Variables] end=[BCs]

The boundary conditions used here are roller boundary conditions, as well as boundary conditions that model the effect of the fluid porepressure on the injection area:

!listing modules/porous_flow/examples/tutorial/04.i start=[BCs] end=[AuxVariables]

The `TensorMechanics` module of MOOSE provides some useful `AuxKernels` for extracting effective stresses of interest to this problem (the effective radial stress and the effective hoop stress)

!listing modules/porous_flow/examples/tutorial/04.i start=[AuxVariables] end=[Modules]

Finally, some mechanics-related `Materials` need to be defined

!listing modules/porous_flow/examples/tutorial/04.i start=[elasticity_tensor] end=[Preconditioning]

An animation of the results is shown in [tut04_gif_fig].

!media porous_flow/tut04.gif style=width:50%;margin-left:10px caption=Displacement (magnified by 100 times) and effective hoop-stress evolution in the borehole-aquifer-caprock system.  id=tut04_gif_fig

The dynamics of this model are fascinating, and readers are encouraged to pause and play with parameters to explore how they effect the final result.  In fact, this model is very similar to the "THM Rehbinder" test in PorousFlow's test suite.  Rehbinder [!citep](rehbinder1995) derived analytical solutions for a similar THM problem, and MOOSE replicates his result exactly:

!media media/porous_flow/thm_rehbinder_temperature_fig.png style=width:60%;margin-left:10px caption=Comparison between MOOSE and Rehbinder's analytical solution.  id=fig_thm_t.fig

!media media/porous_flow/thm_rehbinder_porepressure_fig.png style=width:60%;margin-left:10px caption=Comparison between MOOSE and Rehbinder's analytical solution.  id=fig:thm_p.fig

!media media/porous_flow/thm_rehbinder_displacement_fig.png style=width:60%;margin-left:10px caption=Comparison between MOOSE and Rehbinder's analytical solution.  id=fig:thm_d.fig

!bibtex bibliography


[Start](porous_flow/tutorial_00.md) |
[Previous](porous_flow/tutorial_03.md) |
[Next](porous_flow/tutorial_05.md)

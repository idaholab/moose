# Porous Flow Tutorial Page 06.  Adding a tracer

[Start](porous_flow/tutorial_00.md) |
[Previous](porous_flow/tutorial_05.md) |
[Next](porous_flow/tutorial_07.md)

On this Page we will depart from `PorousFlowBasicTHM` and use `PorousFlowFullySaturated`.  This action employs [mass lumping](porous_flow/mass_lumping.md) but no upwinding.  Therefore, the results will be slightly different than those achieved by `PorousFlowBasicTHM` (no lumping or upwinding) and from the remainder of PorousFlow ([mass lumping](porous_flow/mass_lumping) and [full upwinding](porous_flow/upwinding.md)).

The reason for using `PorousFlowFullySaturated` is that it allows multi-component physics to be studied.  Denoting the mass fraction by $\chi^{\kappa}$ (where $\kappa = 0,\ldots,N_{\mathrm{components}}-1$), the [fluid equations](porous_flow/governing_equations.md) are
\begin{equation}
\label{eq:full_sat}
0 = \frac{\partial}{\partial t} \phi(\rho) \chi^{\kappa} - \nabla_{i} \left(\chi^{\kappa}(\rho)\frac{k_{ij}}{\mu}(\nabla P_{j} - \rho g_{j}) \right) \ .
\end{equation}
In this equation the fluid density, $\rho$, is bracketed, because the user has the option of including it or not using the `multiply_by_density` flag.  The advantage of setting `multiply_by_density = false` is that the equations become more linear, but it means that care must be taken when using other parts of PorousFlow.  For instance, the [`PorousFlowMass`](PorousFlowFluidMass.md) Postprocessor is measuring fluid mass, not fluid volume.

Tracers may now be incorporated into the simulation.  A `tracer_concentration` variable is defined, and set to initial conditions where it is 0.5 at the injection area, and zero elsewhere.  Modifying the isothermal simulation of [Page 01](porous_flow/tutorial_01.md), `PorousFlowFullySaturated` is provided with its name:

!listing modules/porous_flow/examples/tutorial/06.i start=[Variables] end=[BCs]

To encourage flow, rather than a rapid convergence to a more-or-less steady-state situation, the outer boundary is fixed to $P=0$. The tracer concentration is fixed to 0.5 at the injection area.

!listing modules/porous_flow/examples/tutorial/06.i start=[BCs] end=[Modules]

The only other changes are that the `PorousFlowConstantBiotModulus` `Material` is now not needed, and that now a nodal porosity is needed in addition to the one evaluated at the quadpoints, since [eq:full_sat] is lumped to the nodes:

!listing modules/porous_flow/examples/tutorial/06.i start=[Materials] end=[./permeability_aquifer]

If `multiply_by_density = true` then the arguments in [Page 02](porous_flow/tutorial_02.md) concerning the magnitude of the residual, and hence the size of the nonlinear absolute-tolerance must be modified.  Specifically, the fluid residuals now get multiplied by the density (which is the default and most common use-case in PorousFlow).  So in this problem, the fluid residual is approximately
\begin{equation}
R \approx 10^{-8}V\epsilon \ ,
\end{equation}
(since the fluid density is approximately 1000$\,$kg.m$^{-3}$).  The `nl_abs_tol` is therefore

!listing modules/porous_flow/examples/tutorial/06.i start=[Executioner] end=[Outputs]

An animation of the results is shown in [tut06_gif_fig].

!media porous_flow/tut06.gif style=width:50%;margin-left:10px caption=Darcy flow vectors and tracer evolution in the borehole-aquifer-caprock system.  id=tut06_gif_fig

As you gain experience with PorousFlow, you will realise that this simulation contains several defects.

## Boundary conditions

The most important (potential) problem lies with the boundary conditions.  Physically they are saying "add or remove fluid to keep the porepressure or tracer concentration fixed".  In more complex simulations this can be completely disasterous.  There may not be the correct fluid component and fluid phase present to remove, and this causes extremely poor nonlinear convergence.  It is really important to understand this fully.

`PorousFlowFullySaturated` associates the variable $\chi^{\kappa}$ with the mass-balance equation for $\chi^{\kappa}$, and the variable $P$ with the mass-balance equation for the final mass fraction ($1-\sum_{\kappa}\chi^{\kappa}$).  (This is not obvious unless you read the code.  In simulations that don't use `Actions` it will be obvious because you will have to write the input-file `Kernels` block yourself.)  Hence the BCs that have `variable = porepressure`:

!listing modules/porous_flow/examples/tutorial/06.i start=[./constant_injection_porepressure] end=[./injected_tracer]

are saying "add or remove the final mass fraction ($1-\sum_{\kappa}\chi^{\kappa}$) to keep the porepressure fixed"; while the BCs that have `variable = tracer_concentration`:

!listing modules/porous_flow/examples/tutorial/06.i start=[./injected_tracer] end=[]

are saying "add or remove the tracer so that its concentration remains fixed".  In complex multi-phase, multi-component models this can be a real "gotcha" and a [PorousFlowSink](PorousFlowSink.md) is recommended.

## Lack of upwinding

Broadly speaking, the animation shows the expected behaviour, but what are those "stripes" of low and high concentration towards the end of the simulation?  This is caused by the lack of upwinding.  As mentioned in the introduction to this Page, upwinding is standard in most PorousFlow simulations, but is not used here.  In fact, if you run the simulation yourself, you'll see that `tracer_concentration` does not even lie within its expected range of $[0, 0.5]$!  This is removed by upwinding, but comes at a cost: the upwinding introduces extra diffusion, so that sharp fronts are not maintained!  If you are interested in tracking fronts precisely, PorousFlow is probably not the best choice.  The reconstructed discontinuous galerkin module is probably for you.

[Start](porous_flow/tutorial_00.md) |
[Previous](porous_flow/tutorial_05.md) |
[Next](porous_flow/tutorial_07.md)

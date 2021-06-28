# PorousFlowOutflowBC

This adds the following term to the residual
\begin{equation}
\int_{\partial\Omega}\psi {\mathbf n}\cdot \mathbf{F}
\end{equation}
Various forms for $\mathbf{F}$ may be chosen, as discussed next, so that this BC removes fluid species or heat energy through $\partial\Omega$ at exactly the rate specified by the multi-component, multi-phase Darcy-Richards equation, or the heat equation.  Therefore, this BC can be used to represent a "free" boundary through which fluid or heat can freely flow: the boundary is "invisible" to the simulation.  Alternate PorousFlow boundary conditions, such as those allowing boundary fluxes to be prescribed, are discussed [here](boundaries.md).

!alert note
Ensure your normal vector points *out* of the model, otherwise your fluxes will have the wrong sign

!alert warning
`PorousFlowOutflowBC` does not model the interface of the model with "empty space".  Imagine a model of a porous-media pipe containing water.  `PorousFlowOutflowBC` does *not* model the situation where the pipe has an end through which the water flows into empty space.  Instead, `PorousFlowOutflowBC` allows only part of the pipe to be modelled: when water exits the model it continues freely into the unmodelled section of the pipe.  In this sense, the model's boundary is "invisible" to the simulation.  This has a further consequence: if there is a sink in the modelled section, `PorousFlowOutflowBC` will allow water to flow from the unmodelled section into the modelled section.

!alert note
The rate of outflow is limited by the permeability, the viscosity of the fluid, etc, in exactly the same way that the Darcy velocity is limited by these quantities.  This means, for instance, if you inject a lot of fluid or heat into the model, it will take the `PorousFlowOutflowBC` some time to "suck" it all out.

Worked examples of this boundary condition may be found in the [boundaries documentation](boundaries.md).

This BC is fully upwinded, so can be used in conjunction with the [PorousFlowAdvectiveFlux](PorousFlowAdvectiveFlux.md), [PorousFlowHeatAdvection](PorousFlowHeatAdvection.md) and [PorousFlowHeatConduction](PorousFlowHeatConduction.md) Kernels.

### Fluid flow

To allow a fluid species, $\kappa$, to flow freely out of the model, the [governing equations](governing_equations.md) imply that
\begin{equation}
\mathbf{F} = \sum_{\beta}\chi_{\beta}^{\kappa}\mathbf{F}_{\beta}^{\mathrm{advective}} + \mathbf{F}^{\kappa}_{\mathrm{diffusion+dispersion}} \ ,
\end{equation}
where the standard [Porous Flow nomenclature](/porous_flow/nomenclature.md) has been used.  Many PorousFlow simulations do not involve diffusion and dispersion, so the latter term is not included in `PorousFlowOutflowBC`: $\mathbf{F}$ is simply
\begin{equation}
\mathbf{F} = \sum_{\beta}\chi_{\beta}^{\kappa}\mathbf{F}_{\beta}^{\mathrm{advective}} = -\sum_{\beta}\chi_{\beta}^{\kappa}\rho_{\beta}\frac{k\,k_{\mathrm{r,}\beta}}{\mu_{\beta}}(\nabla
P_{\beta} - \rho_{\beta} \mathbf{g}) \ .
\end{equation}

Input-file syntax for this type of boundary condition requires specifying:

- `flux_type = fluid` (the default);
- the `mass_fraction_component` $\kappa$;
- the flag `include_relperm`, which should only be set `false` in fully-saturated situations where there is no notion of relative permeability;
- the flag `multiply_by_density`, which means the above expression is *not* premultiplied by $\rho_{\beta}$.  This allows the `PorousFlowOutflowBC` to be used with other objects that do not multiply by density in simulations based on fluid volume instead of fluid mass.

### Heat flow

To allow heat energy to flow freely out of the model, the [governing equations](governing_equations.md) imply that
\begin{equation}
\mathbf{F} = -\lambda \nabla T + \sum_{\beta}h_{\beta}\mathbf{F}_{\beta} = -\lambda \nabla T - \sum_{\beta}h_{\beta}\rho_{\beta}\frac{k\,k_{\mathrm{r,}\beta}}{\mu_{\beta}}(\nabla
P_{\beta} - \rho_{\beta} \mathbf{g}) \ .
\end{equation}

Input-file syntax for this type of boundary condition requires specifying:

- `flux_type = heat`;
- the flag `include_relperm`, which should only be set `false` in fully-saturated situations where there is no notion of relative permeability.

In this case, `multiply_by_density` is set to `true` irrespective of the choice set by the user, since it is an error not to multiply by density.  Any `mass_fraction_component` specified is ignored.


### Example: single-phase, single-component fluid-flow through a boundary

The most basic usage of [PorousFlowOutflowBC](PorousFlowOutflowBC.md) is illustrated in the following:

!listing modules/porous_flow/test/tests/sinks/s14.i start=[BCs] end=[Preconditioning]

In this input-file, all boundaries are of the "outflow" type, and the total flow rate (kg.s$^{-1}$) is recorded into the `outflow_kg_per_s` using a [NodalSum](NodalSum.md) postprocessor.


### Example: heat flow through a boundary

A [PorousFlowOutflowBC](PorousFlowOutflowBC.md) with `flux_type = heat` will allow heat to flow through a boundary.  To record the total heat-energy flowing through the boundary a [NodalSum](NodalSum.md) postprocessor should be used:

!listing modules/porous_flow/test/tests/sinks/s15.i start=[BCs] end=[Preconditioning]


## A multicomponent example

A fully-saturated 2-component modelled on a 1D line segment, $0\leq x \leq 1$:

!listing sinks/s13.i block=PorousFlowFullySaturated

The model initializes with the porous material fully filled with component = 1 (that is, `frac = 0`) and with a porepressure gradient that will move fluid from negative $x$ to positive $x$:
\begin{equation}
p(t=0) = 1 - x \ \ \ \mathrm{and}\ \ \ \chi^{0}(t=0) = 0 \ .
\end{equation}

!listing sinks/s13.i block=Variables

!listing sinks/s13.i block=ICs

For $t>0$, fluid component $0$ (the `frac` variable) is introduced on the material's left
side ($x=0$), by applying the fixed boundary conditions:
\begin{equation}
p(x=0) = 1 \ \ \ \mathrm{and}\ \ \ \chi^{0}(x=0) = 1 \ .
\end{equation}
The right-hand side, at $x=1$, has fixed porepressure
\begin{equation}
p(x=1) = 0 \ .
\end{equation}
while the fluid component $0$ (the `frac` variable) is allowed to freely exit the domain when it arrives there.

- Porepressure is held at `pp = 1` at $x=0$.  (Physically this adds or removes component = 1 to ensure that porepressure is fixed.)
- Porepressure is held at `pp = 0` at $x=1$.  (Physically this adds or removes component = 1 to ensure that porepressure is fixed.)  These two BCs ensure the porepressure gradient is maintained.
- The zeroth mass fraction is held at `frac = 1` at $x=0$.
- The zeroth mass fraction is allowed to freely exit the model at $x=1$.

The former 3 are implemented using [DirichletBC](DirichletBC.md), while the latter is the `PorousFlowOutflowBC`.  It acts on `mass_fraction_component = 0` because that is the mass-fraction associated to the `frac` variable by the [PorousFlowFullySaturated Action](PorousFlowFullySaturated.md):

!listing sinks/s13.i block=BCs

The results are shown in [s13] where it can be seen the fluid component freely exits the right-hand boundary.

!media sinks/s13.png
	style=width:60%;margin:auto;padding-top:2.5%;
	id=s13
	caption=Advection and diffusion of a fluid component from left to right along a porepressure gradient.  The fluid component is free to exit the right-hand boundary due to the PorousFlowOutflowBC there.

Further description of this result and further examples may be found in the [boundaries documentation](boundaries.md) and [sinks tests documentation](sinks_tests.md).


!syntax parameters /BCs/PorousFlowOutflowBC

!syntax inputs /BCs/PorousFlowOutflowBC

!syntax children /BCs/PorousFlowOutflowBC

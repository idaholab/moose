[Start](porous_flow/tutorial_00.md) |
[Previous](porous_flow/tutorial_06.md) |
[Next](porous_flow/tutorial_08.md)

# Porous Flow Tutorial Page 07.  A chemically-reactive tracer, and porosity and permeability changes

In this Page, the model from [Page 06](porous_flow/tutorial_06.md) is enhanced so that its tracer is chemically reactive.  PorousFlow has the ability to simulate [aqueous equilibrium chemistry](PorousFlowMassFractionAqueousEquilibriumChemistry.md) and [aqueous precipitation-dissolution chemistry](PorousFlowAqueousPreDisChemistry.md).

As an illustration, in this Page, the tracer is assumed to precipitate according to

\begin{equation}
\label{eq:predis}
\chi \rightleftharpoons  M
\end{equation}

where $\chi$ is the tracer concentration (m$^{3}$(tracer)/m$^{3}$(solution)) $M$ is a mineral (m$^{3}$(mineral)/m$^{3}$(porous-material)).  This precipitation causes [porosity changes](PorousFlowPorosity.md) and [permeability changes](PorousFlowPermeabilityKozenyCarman.md).

The input file of [Page 06](porous_flow/tutorial_06.md) is modified to include chemistry.  The first modification is to the [PorousFlowFullySaturated](PorousFlowFullySaturated.md) so that it understands there is one chemical reaction:

!listing modules/porous_flow/examples/tutorial/07.i start=[PorousFlowFullySaturated] end=[AuxVariables]

Setting the `temperature` here allows control of the chemical reaction rate (which is temperature dependent: see [chemical reactions](/chemical_reactions/index.md)).

As precipitation or dissolution occurs via [eq:predis],
the tracer concentration gets decreased or increased.  Thus, a new `Kernel` must be added, which is a [`PorousFlowPreDis`](PorousFlowPreDis.md) Kernel:

!listing modules/porous_flow/examples/tutorial/07.i start=[Kernels] end=[BCs]

The chemical reaction rate is computed by a [`PorousFlowAqueousPreDisChemistry`](PorousFlowAqueousPreDisChemistry.md) `Material`:

!listing modules/porous_flow/examples/tutorial/07.i start=[precipitation_dissolution_mat] end=[mineral_concentration]

All the parameters are fully explained in the [chemical reactions](/chemical_reactions/index.md) module.  Briefly, in this case, because the `reference_temperature` equals the `temperature` specified in the `PorousflowFullySaturated`, there is no temperature dependence of the reaction rate, so it is just the product of the `kinetic_rate_constant` ($10^{-8}\,$mol.m$^{-2}$.s$^{-1}$), the `specific_reactive_surface_area` (1$\,$m$^{2}$.L$^{-1}$), the molar volume (1$\,$L.mol$^{-1}$) and $1 - \chi/K$, where $K$ is the equilibrium constant (0.1):
\begin{equation}
\mathrm{reaction rate} = 10^{-8}(1 - 10\chi) \ \mathrm{L(precipitate)/L(solution)/s} \ .
\end{equation}
This `Material` feeds its rate into the `PorousFlowPreDis` `Kernel` (to alter the tracer concentration), as well as into a [`PorousFlowAqueousPreDisMineral`](PorousFlowAqueousPreDisMineral.md) `Material`:

!listing modules/porous_flow/examples/tutorial/07.i start=[mineral_concentration] end=[]

The reason for this `Material` is that we can build an `AuxVariable` (below) to record the concentration of the precipitated mineral, but also because the mineral concentration enters into the [porosity](/porous_flow/porosity.md) calculation.  The porosity material is

!listing modules/porous_flow/examples/tutorial/07.i start=[porosity_mat] end=[permeability_aquifer]

In PorousFlow, the permeability may depend on the porosity via the [KozenyCarman](PorousFlowPermeabilityKozenyCarman.md) relationship.  In this case, the `Materials` look like:

!listing modules/porous_flow/examples/tutorial/07.i start=[permeability_aquifer] end=[precipitation_dissolution_mat]

Instead of using a set of preset `DirichletBC` to control the physics at the injection area, tracer is simply injected using a [`PorousFlowSink`](PorousFlowSink.md) (see also [boundary conditions](porous_flow/boundaries.md) for a detailed discussion).  A fixed rate of $5\times 10^{-3}\,$kg.s$^{-1}$.m$^{-2}$ is used:

!listing modules/porous_flow/examples/tutorial/07.i start=[BCs] end=[Modules]

Finally, a set of `AuxVariables` (some employing the handy [`PorousFlowPropertyAux`](PorousFlowPropertyAux.md)) is used to record useful information:

!listing modules/porous_flow/examples/tutorial/07.i start=[AuxVariables] end=[Kernels]

An animation of the results is shown in [tut07_gif_fig].

!media porous_flow/tut07.gif style=width:80%;margin-left:10px caption=Tracer concentration, porepressure, mineral concentration and permeability evolution in the borehole-aquifer-caprock system.  id=tut07_gif_fig

The simulation may be promoted to a full THMC simulation using the approach used in [Page 03](porous_flow/tutorial_03.md) and [Page 04](porous_flow/tutorial_04.md).  The arguments made about scaling the variables must be modified to take into account the fluid density appearing in the fluid equation (see [Page 06](porous_flow/tutorial_06.md)) so the scaling will be approximately $10^{-5}$ for the temperature and $10^{-7}$ for the displacement variables.  The [porosity](/porous_flow/porosity.md) may depend on porepressure, temperature, volumetric strain and chemistry.

[Start](porous_flow/tutorial_00.md) |
[Previous](porous_flow/tutorial_06.md) |
[Next](porous_flow/tutorial_08.md)

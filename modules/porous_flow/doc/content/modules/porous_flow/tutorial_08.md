[Start](porous_flow/tutorial_00.md) |
[Previous](porous_flow/tutorial_07.md) |
[Next](porous_flow/tutorial_09.md)

# Porous Flow Tutorial Page 08.  The `PorousFlowSink` and unsaturated flow

The [`PorousFlowSink`](porous_flow/boundaries.md) has been mentioned in this tutorial already, but now is an appropriate time to study it more carefully.  It is a very powerful BC which can handle virtually all physical situations.  It also helps enormously in resolving some [convergence problems](porous_flow/nonlinear_convergence_problems.md).  Therefore, it is well worth studying, so please feel free to take a break from this tutorial and read the info [here](porous_flow/boundaries.md).

Now that you've finished studying the `PorousFlowSink` let's apply it in a simple situation.  The purely fluid-flow model of [Page 01](porous_flow/tutorial_01.md) is extended to include unsaturated flow.  Firstly, the `PorousFlowBasicTHM` `Action` must be replaced by a [PorousFlowUnsaturated](actions/PorousFlowUnsaturated.md) `Action`:

!listing modules/porous_flow/examples/tutorial/08.i start=[PorousFlowUnsaturated] end=[BCs]

There are evidently some more input parameters:

- The `relative_permeability` parameters.  These are documented in
  [Relative permeability](porous_flow/relative_permeability.md)

- The `van_genuchten` capillarity parameters.  These are documented in
  [Capillary pressure](porous_flow/capillary_pressure.md)

The [`PorousFlowConstantBiotModulus`](PorousFlowConstantBiotModulus.md) is not needed here (it's only needed by `PorousFlowBasicTHM`).  Because of the multiplication of the fluid mass-balance equation by density, an appropriate `nl_abs_tol` is $10^{-7}$ in contrast to the $10^{-10}$ calculated on [Page 02](porous_flow/tutorial_02.md):

!listing modules/porous_flow/examples/tutorial/08.i start=[Executioner] end=[Outputs]

Finally we get to the `PorousFlowSink`.  In the following, fluid is extracted through injection_area at a constant rate of $10^{-2}\,$kg.s$^{-1}$.m$^{-2}$.  However, in the unsaturated region ($P<0$) this rate is modified by the relative permeability because `use_relperm = true`.  This greatly improves convergence and in many cases is physically reasonable as there are limits to pumps and other similar machinery.

!listing modules/porous_flow/examples/tutorial/08.i start=[BCs] end=[Modules]

An animation of the results is shown in [tut08.gif.fig].

!media porous_flow/tut08.gif style=width:50%;margin-left:10px caption=Porepressure evolution in the production version of the borehole-aquifer-caprock system.  id=tut08.gif.fig

Try to run the simulation with `use_relperm = false` and you'll see very poor convergence.  The total fluid extracted is $10^{-2}\times 4\pi/2 \times 10^{6} \approx 10^{5}\,$kg (the product of rate, area and total time), while the model contains $0.1 \times 1000 \times 12\pi(10^{2} - 1^{2}) = 4\times 10^{5}\,$kg (the product of porosity, fluid density, and model volume), so the model is not running completely dry, but it requires extremely low porepressures to affect the constant rate of fluid removal ($P$ gets so low that MOOSE starts to suffer from precision loss).

The simulation may be promoted to a full THMC simulation using the approach used in [Page 03](porous_flow/tutorial_03.md), [Page 04](porous_flow/tutorial_04.md) and [Page 06](porous_flow/tutorial_06.md).  The arguments made about scaling the variables must be modified to take into account the fluid density appearing in the fluid equation (see [Page 06](porous_flow/tutorial_06.md)) so the scaling will be approximately $10^{-5}$ for the temperature and $10^{-7}$ for the displacement variables.  The [porosity](/porous_flow/porosity.md) may depend on porepressure, temperature, volumetric strain and chemistry.

The simulation described so far uses [full upwinding](upwinding.md), which is the default in PorousFlow.  [TVD stabilization](kt.md) (see also [numerical diffusion](numerical_diffusion.md) and [a worked example of KT stablization](kt_worked.md)) may be used instead by simply changing the `PorousFlowUnsaturated` block to:

!listing modules/porous_flow/examples/tutorial/08_KT.i start=[PorousFlowUnsaturated] end=[BCs]


[Start](porous_flow/tutorial_00.md) |
[Previous](porous_flow/tutorial_07.md) |
[Next](porous_flow/tutorial_09.md)

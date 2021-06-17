[Start](porous_flow/tutorial_00.md) |
[Previous](porous_flow/tutorial_09.md) |
[Next](porous_flow/tutorial_11.md)

# Porous Flow Tutorial Page 10.  Unleashing the full power of PorousFlow: using Kernels and Materials

Now we're ready to build an input file from scratch using `Kernels` and `Materials` instead of the `Actions` we've been using so far.  Inevitably you'll have to do this yourself when running PorousFlow simulations: you'll always want some PorousFlow features (boundary conditions, line sources, postprocessors, AuxKernels, etc) that aren't built by the `Actions`.

We're going to build the simulation from [Page 08](porous_flow/tutorial_08.md) which involved unsaturated flow.  After building the input file, you can compare it with the `Action` version.  The mesh and `Variables` remain unchanged.  For clarity later on, the porepressure variable has been renamed 'pp'

!listing modules/porous_flow/examples/tutorial/10.i start=[Variables] end=[Kernels]

Let's start by building the [`PorousFlowDictator`](PorousFlowDictator.md).  We have one phase, one component and no chemistry.  The single PorousFlow variable is called pp:

!listing modules/porous_flow/examples/tutorial/10.i start=[UserObjects] end=[pc]

It is always useful to put the name of the `PorousFlowDictator` in the `GlobalParams` block, since all PorousFlow objects need it:

!listing modules/porous_flow/examples/tutorial/10.i start=[GlobalParams] end=[Variables]

The DE is unsaturated single-phase flow.  Refer to the [governing equations](porous_flow/governing_equations.md) document.  Unsaturated single-phase flow is described by the first equation without the coupling terms to solid mechanics, radioactive decay, chemistry and sources:

\begin{equation}
\label{eq:mass_cons_sp}
0 = \frac{\partial M^{\kappa}}{\partial t} + \nabla\cdot \mathbf{F}^{\kappa}  \ .
\end{equation}

Here $\kappa = 0$ since there is just one fluid component.  The fluid mass is $M = \phi S \rho$ and the fluid velocity is $F_{i} = -\rho k_{ij} (\nabla_{j} P - \rho g_{j}) / \mu$.  Hence we have two `Kernels` (refer to the bottom of [governing equations](porous_flow/governing_equations.md) for their types)

!listing modules/porous_flow/examples/tutorial/10.i start=[Kernels] end=[AuxVariables]

It is often common to define some `AuxVariables` for visualisation purposes (or to feed as coupled variables to other MOOSE objects).  A useful variable here is the fluid saturation, which is computed using a [`PorousFlowPropertyAux`](PorousFlowPropertyAux.md) so the variable must be a constant monomial:

!listing modules/porous_flow/examples/tutorial/10.i start=[AuxVariables] end=[BCs]

The `BCs` remain the same: they used a [`PorousFlowSink`](porous_flow/boundaries.md) to withdraw fluid from the injection area.  The fluid properties also remain the same.  For reference, these two blocks are:

!listing modules/porous_flow/examples/tutorial/10.i start=[BCs] end=[Materials]

The final block to create is the `Materials`.  This is always the most complicated part of creating an input file, and really only comes by experience.

- You can run MOOSE multiple times, each time getting "undefined"
  errors like the one at the top of [Page 09](porous_flow/tutorial_09.md) and slowly add the required
  Materials until you get no errors.  The information near the bottom
  of [Page 09](porous_flow/tutorial_09.md) is useful here.

- You can inspect other similar input files in the PorousFlow test
  suite.  There are over 300 tests so you're very likely to find what
  you need.  Don't expect the files to always correspond to
  physically-sensible models (sometimes they're deliberately
  unsensible to test obscure aspects of PorousFlow) but at least they
  are guaranteed to run!

- You can ask on the [MOOSE Discussion forum](https://github.com/idaholab/moose/discussions).

In this case, the DEs involve porosity, fluid saturation, fluid density, permeability and viscosity.  The fluid mass is lumped to the nodes, so we'll only need porosity at the nodes:

!listing modules/porous_flow/examples/tutorial/10.i start=[porosity] end=[permeability_aquifer]

The permeability is also needed:

!listing modules/porous_flow/examples/tutorial/10.i start=[permeability_aquifer] end=[saturation_calculator]

These `Materials` were also in the model of [Page 08](porous_flow/tutorial_08.md).

Now we need to build the saturation.  This is *not* computed by the `PorousFlowPropertyAux` above.  That `AuxKernel` just retrieves the material property and puts it into an `AuxVariable`.  As mentioned in [Page 09](porous_flow/tutorial_09.md) PorousFlow simulations can use a variety of primary `Variables`.  *A set of fundamental Materials computes porepressures, saturations, temperatures and mass fractions from these primary Variables*.  In this case, our primary Variable is just porepressure and we only need to compute a single saturation.  However, various other PorousFlow objects need temperature as an input, so we must compute it too, even though we just set it to a constant 293.  Also, mass fraction needs to be computed, although it is trivially 1.0 for this problem.

!listing modules/porous_flow/examples/tutorial/10.i start=[saturation_calculator] end=[simple_fluid]

These are our so-called "fundamental Materials".  In almost every PorousFlow simulation you will see similar `Materials`.  The saturation calculator uses the capillary-pressure function.  It is a `UserObject`:

!listing modules/porous_flow/examples/tutorial/10.i start=[UserObjects] end=[GlobalParams]

The density and viscosity need to be computed.  This is achieved by a [`PorousFlowSingleComponentFluid`](PorousFlowSingleComponentFluid.md)

!listing modules/porous_flow/examples/tutorial/10.i start=[simple_fluid] end=[relperm]

Finally, the relative permeability must be defined for each phase (just one phase in this example)

!listing modules/porous_flow/examples/tutorial/10.i start=[relperm] end=[]

Our input file has been built!  You may check that it gives exactly the same answers as the one on [Page 08](porous_flow/tutorial_08.md).  It is 232 lines long while the one on Page 08 is only 139 lines long.  Now that you've reached this point, you can start to build much more powerful models that don't use the PorousFlow `Actions`: models that involve multi-phase, multi-component flows, complicated thermal couplings, elasto-plasticity, chemical reactions, line sources and sinks, and sophisticated boundary terms!

[Start](porous_flow/tutorial_00.md) |
[Previous](porous_flow/tutorial_09.md) |
[Next](porous_flow/tutorial_11.md)

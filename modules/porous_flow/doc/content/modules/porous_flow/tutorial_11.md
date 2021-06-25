[Start](porous_flow/tutorial_00.md) |
[Previous](porous_flow/tutorial_10.md) |
[Next](porous_flow/tutorial_12.md)

# Porous Flow Tutorial Page 11.  Two-phase THM borehole injection

Hold onto your hats, here we're going to build a two-phase THM simulation from scratch.  Our starting point is the borehole-reservoir-caprock geometry developed in [Page 00](porous_flow/tutorial_00.md).

- We would like to simulate cold CO$_{2}$ injection into an initially
  water-filled reservoir.

- There are two phases, and two components.  The liquid phase is
  filled with water only, while the "gas" phases is filled with
  CO$_{2}$ only ("gas" is in quotes because it may be supercritical).

- There is capillarity between the water and CO$_{2}$.

- The fluids have certain nontrivial relative permeability curves.

- Heat is advected with the fluids as well as conducted through the
  fluid-rock system.

- The rock can deform elastically.

- There is no gravity (for simplicity as this effects initial stresses
  and fluid pressures, which is not difficult to include but the input
  file becomes even longer).

- Full coupling between the fluids, the temperature and the
  displacements is used.

- The porosity and permeability are functions of the porepressures,
  saturations, temperature and displacements

- High precision equations of state are used for the water and
  CO$_{2}$.

- At the injection area, CO$_{2}$ is injected at a constant rate and
  at a constant temperature.  It pushes mechanically against the
  borehole wall

- At the outer boundary fluid is removed slowly using a
  `PorousFlowSink`.

- In this model there are no vertical displacements


## Variables

Let's step through the input file.  This simulation uses the water and gas porepressures as the independent fluid pressures.  A perfectly valid alternative would be to use water saturation and gas porepressure.  A scaling factor is needed for the temperature and displacement variables as discussed in the [convergence page](porous_flow/convergence.md).  The `Variables` are given initial conditions appropriate to a deep CO$_{2}$ sequestration site:

!listing modules/porous_flow/examples/tutorial/11.i start=[Variables] end=[Kernels]

## Dictator

The `PorousFlowDictator` is given the variable names as well as the number of fluid phases and components.

!listing modules/porous_flow/examples/tutorial/11.i start=[dictator] end=[pc]

## GlobalParams

The global parameters consist of various parameters that are required by several PorousFlow objects, and using `GlobalParams` allows the input file to remain organised and a little more succinct:

!listing modules/porous_flow/examples/tutorial/11.i start=[GlobalParams] end=[Variables]

## Kernels

The `Kernels` block is rather big.  Please refer to the [governing equations page](porous_flow/governing_equations.md) to see what equations are being modelled here.  Virtually all the power of PorousFlow is being used.  The only things that are missing are the following.

- Chemical reactions and desorption are not used.

- Fluid diffusion and dispersion is not active because water only
  exists in the liquid phase and CO$_{2}$ in the gas phase.

- There is no radioactive decay.

- There is no plasticity, which also means there is no plastic
  heating.

- The solid mechanics is assumed quasi-static.

The `Kernels` block is

!listing modules/porous_flow/examples/tutorial/11.i start=[Kernels] end=[AuxVariables]

## AuxVariables and AuxKernels

Some `AuxVariables` are defined that need further explanation.

- The solid mechanics needs 3 displacement variables.  In the
  assumptions above, the vertical displacement is always zero.  The
  best way of defining it is as an `AuxVariable` without an
  `AuxKernel` so that it will always be zero, but then it may be
  coupled into various MOOSE objects (using the `displacements =` in the `GlobalParams`)

- CO$_{2}$ is pumped into this model with a constant rate.  To achieve
  this, the CO$_{2}$ pressure in the borehole must gradually increase
  over time.  This means the fluid in the borehole will push against
  the borehole wall with increasing pressure.  Hence this pressure
  must be recorded and coupled into the mechanical `BCs`.  It is
  recorded in the `effective_fluid_pressure` `AuxVariable`.

- The mass fractions of each component in each phase must be defined,
  even if they are fixed for always.  Since they are unchanging they
  are most conveniently represented by `AuxVariables` with certain
  initial conditions and no `AuxKernels`.

- The other `AuxVariables` are useful for visualising the results.

!listing modules/porous_flow/examples/tutorial/11.i start=[AuxVariables] end=[BCs]

## Boundary conditions

The boundary conditions for the displacements are roller on the sides, fixed at the top and bottom (an arbitrary choice made by the creator of this input file) and `Pressure` boundary conditions on the injection_area:

!listing modules/porous_flow/examples/tutorial/11.i start=[roller_tmax] end=[cold_co2]

Notice the `constrained_effective_fluid_pressure_at_wellbore`.  This is almost the `effective_fluid_pressure` `AuxVariable` defined above, evaluated at the injection_area.  But there is a problem at the first timestep, because this uses   Material properties that are not properly initialised.  So a little bit of deception is used.  Firstly, the `AuxVariable` is evaluated at the injection_area and put into a `Postprocessor`:

!listing modules/porous_flow/examples/tutorial/11.i start=[Postprocessors] end=[constrained_effective_fluid_pressure_at_wellbore]

Then a `Function` is made that returns either the value of this `Postprocessor` or 20E6 (the initial reservoir pressure)

!listing modules/porous_flow/examples/tutorial/11.i start=[Functions] end=[Preconditioning]

Finally, the value of this `Function` is placed into the `Postprocessor` used in the Pressure BC:

!listing modules/porous_flow/examples/tutorial/11.i start=[constrained_effective_fluid_pressure_at_wellbore] end=[]

The boundary conditions for temperature is a simple preset `DirichletBC` on the injection_area:

!listing modules/porous_flow/examples/tutorial/11.i start=[cold_co2] end=[constant_co2_injection]

The boundary conditions for the fluids at the injection_area is just a constant injection of CO$_{2}$ with rate $10^{-4}\,$kg.s$^{-1}$.m$^{-2}$:

!listing modules/porous_flow/examples/tutorial/11.i start=[constant_co2_injection] end=[outer_water_removal]

At the outer boundary, water and CO$_{2}$ are removed if their porepressures rise above their initial values.  A `PorousFlowPiecewiseLinearSink` is used, with an imaginary boundary at fixed porepressure sitting at a distance of $L=10\,$m outside the model.  The procedure of constructing this sink is described fully in the [boundaries documentation](porous_flow/boundaries.md).  The input-file blocks are

!listing modules/porous_flow/examples/tutorial/11.i start=[outer_water_removal] end=[Modules]

## Fluid properties

High-precision equations of state are used for both the water and the CO$_{2}$.  Before running the simulation, these are tabulated, and the tabulated versions are used by MOOSE in all computations, which shortens the simulation time:

!listing modules/porous_flow/examples/tutorial/11.i start=[Modules] end=[Materials]

## Materials

The capillary pressure relationship is defined by the `UserObject`:

!listing modules/porous_flow/examples/tutorial/11.i start=[pc] end=[]

As explained on [Page 09](porous_flow/tutorial_09.md) and [Page 10](porous_flow/tutorial_10.md), there are a set of "fundamental Materials" that compute all porepressures, saturations, temperature and mass fractions as Material properties (as well as their gradients, and the derivatives with respect to the primary `Variables`, etc).  In the case at hand, these fundamental Materials are:

!listing modules/porous_flow/examples/tutorial/11.i start=[Materials] end=[water]

The water and CO$_{2}$ densities, viscosities, enthalpies, and internal energies (as well as derivatives of these) are computed by

!listing modules/porous_flow/examples/tutorial/11.i start=[water] end=[relperm_water]

A Corey type of [relative permeability](porous_flow/relative_permeability.md) is chosen for the liquid phase, and a Brooks-Corey type of relative permeability is chosen for the gas phase:

!listing modules/porous_flow/examples/tutorial/11.i start=[relperm_water] end=[porosity]

[Porosity](/porous_flow/porosity.md) is chosen to depend on porepressures, saturations (actually just the effective porepressure), temperature and mechanical strain using:

!listing modules/porous_flow/examples/tutorial/11.i start=[porosity_mat] end=[permeability_aquifer]

Permeability is chosen to follow the Kozeny-Carman relationship:

!listing modules/porous_flow/examples/tutorial/11.i start=[permeability_aquifer] end=[rock_thermal_conductivity]

The rock thermal conductivity is chosen to be independent of water saturation and to be isotropic (and independent of rock type --- reservoir or cap-rock):

!listing modules/porous_flow/examples/tutorial/11.i start=[rock_thermal_conductivity] end=[rock_internal_energy]

while the rock internal energy is also constant:

!listing modules/porous_flow/examples/tutorial/11.i start=[rock_internal_energy] end=[elasticity_tensor]

The elasticity tensor of the rock (both reservoir and cap-rock) is assumed isotropic with a Young's modulus of 5$\,$GPa:

!listing modules/porous_flow/examples/tutorial/11.i start=[elasticity_tensor] end=[strain]

The strain calculator must take into consideration both the thermal strain (see [governing equations](porous_flow/governing_equations)) as well as initial effective stress.  The initial total stress is assumed to be zero (for simplicity, not because it is physically very likely, but a nonzero value doesn't change the results much), so the initial effective stress is just the initial porepressure

!listing modules/porous_flow/examples/tutorial/11.i start=[strain] end=[stress]

The `thermal_contribution` eigenstrain name has to be provided to the `StressDivergenceTensors` `Kernels`, by the way (see above).  Stress is just linear elastic:

!listing modules/porous_flow/examples/tutorial/11.i start=[stress] end=[effective_fluid_pressure_mat]

Finally, the effective fluid pressure must be computed because it is needed by the Porosity and the solid-fluid coupling, and the volumetric strain feeds into the Porosity:

!listing modules/porous_flow/examples/tutorial/11.i start=[effective_fluid_pressure_mat] end=[Postprocessors]

## Executioner

For this model, an `IterationAdaptiveDT` `Timestepper` is used.  This is because the dynamics at early times, particularly the thermal shock induced by sudden application of cool CO$_{2}$ at the injection area, means small timesteps are needed, but after some time larger timesteps can be used.

!listing modules/porous_flow/examples/tutorial/11.i start=[Executioner] end=[Outputs]

## The end

Holey Dooley, you made it to the end, well done!

An animation of the results is shown in [tut11.gif.fig].  The porepressure front moves relatively quickly, followed by the saturation front, and then the temperature front.  The solid mechanical deformations are governed mostly by the temperature.  By the way, this type of dynamic 2-phase injection problem using PorousFlow has been benchmarked against analytical results with excellent agreement (and will hopefully be written into a PorousFlow Example --- we are awaiting permission by funding bodies).

!media porous_flow/tut11.gif style=width:90%;margin-left:10px caption=CO$_{2}$ saturation, CO$_{2}$ porepressure, temperature and hoop stress in the 2-phase CO$_{2}$ injection simulation.  id=tut11.gif.fig

## Postscript: the same again in 2D

As mentioned on [Page 00](porous_flow/tutorial_00.md), this problem is really an axially-symmetric problem, so may be better modelled by MOOSE using its "RZ" coordinate system.  The changes to the input file are minimal.  Apart from the mesh generation (discussed in [Page 00](porous_flow/tutorial_00.md)) the changes are itemized below.

There only need by a `disp_r` Variable in place of `disp_x` and `disp_y`:

!listing modules/porous_flow/examples/tutorial/11_2D.i start=[disp_r] end=[]

!listing modules/porous_flow/examples/tutorial/11_2D.i start=[GlobalParams] end=[]

!listing modules/porous_flow/examples/tutorial/11_2D.i start=[dictator] end=[pc]

There are mechanical Kernels only for `disp_r`, and the `StressDivergenceTensors` Kernel is modified:

!listing modules/porous_flow/examples/tutorial/11_2D.i start=[grad_stress_r] end=[poro_r]

The stress `AuxKernels` are modified.  In TensorMechanics with RZ coordinates, the 00 component is $rr$, the 11 component is $zz$ and the 22 component is $\theta\theta$.  Therefore, these `AuxKernels` read

!listing modules/porous_flow/examples/tutorial/11_2D.i start=[stress_rr_aux] end=[porosity]

The boundary conditions for the mechanics become simpler:

!listing modules/porous_flow/examples/tutorial/11_2D.i start=[pinned_top_bottom_r] end=[cold_co2]

Finally, the strain calculator needs to be of RZ type:

!listing modules/porous_flow/examples/tutorial/11_2D.i start=[strain] end=[thermal_contribution]

The reader may check that the 3D and 2D models produce the same answer, although of course the 2D version is much faster due to it having only 176 degrees of freedom compared with the 3D's 1100.


[Start](porous_flow/tutorial_00.md) |
[Previous](porous_flow/tutorial_10.md) |
[Next](porous_flow/tutorial_12.md)

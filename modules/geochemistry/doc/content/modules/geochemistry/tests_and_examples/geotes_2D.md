# Simulating a simple GeoTES experiment

This page describes a reactive-transport simulation of a simple GeoTES scenario.  The transport is handled using the PorousFlow module, while the Geochemistry module is used to simulate a heat exchanger and the geochemistry.  The simulations are coupled together in an operator-splitting approach using MultiApps.

In Geological Thermal Energy Storage (GeoTES), hot fluid is pumped into an subsurface confined aquifer, stored there for some time, then pumped back again to produce electricity.  Many different scenarios have been proposed, including different borehole geometries (used to pump the fluid), different pumping strategies and cyclicities (pumping down certain boreholes, up other boreholes, etc), different temperatures, different aquifer properties, etc.  This page only describes the heating phase of a particularly simple set up.

Two wells (boreholes) are used.

1. The cold (production) well, which pumps aquifer water from the aquifer.
2. The hot (injection) well, which pumps hot fluid into the aquifer.

These are connected by a heat-exchanger, which heats the aquifer water produced from (1) and passes it to (2).  This means the fluid pumped through the injection well is, in fact, heated aquifer water, which is important practically from a water-conservation standpoint, and to assess the geochemical aspects of circulating aquifer water through the GeoTES system.

!media geotes_2D_schematic.png caption=Schematic of the simple GeoTES simulation.  id=geotes_2D_schematic.fig

## Aquifer geochemistry

It is assumed that only H$_{2}$O, Na$^{+}$, Cl$^{-}$ and SiO$_{2}$(aq) are present in the system.  While this is unrealistically simple for any aquifer, it allows this page to focus on the method of constructing more complicated models avoiding rather lengthy input files.

### The database file

A small geochemical database involving only these species is used.  Readers who are building their own database might find exploring its structure useful:

!listing modules/combined/examples/geochem-porous_flow/geotes_2D/small_database.json

### Initial conditions

The aquifer temperature is uniformly 50$^{\circ}$C before injection and production.

The aquifer is assumed to be consist entirely of a mineral called `QuartzLike` and a kinetic rate is used for its dissolution.  The mineral `QuartzLike` is similar to `Quartz` but it has a different equilibrium constant structure in order to emphasise the dissolution upon heating.

Each 1$\,$kg of solvent water is assumed to hold 0.1$\,$mol of Na$^{+}$ and Cl$^{-1}$, and a certain amount of SiO$_{2}$(aq).  The following input file finds the free molality of SiO$_{2}$(aq) when in contact with `QuartzLike` at 50$^{\circ}$C:

!listing modules/combined/examples/geochem-porous_flow/geotes_2D/aquifer_equilibrium.i

The result it `molality = 0.000555052386`.  It is necessary to find this initial molality, only because `QuartzLike` is a kinetic species, so cannot appear the basis (see another [similar example](kinetic_quartz_arrhenius.md)).  If it were treated as an equilibrium species, its initial free mole number could be specified in the input file below (see [an example of this](gypsum.md)).

### Spatially-dependent geochemistry simulation

Because an operator-splitting approach is used, the simulation of the aquifer geochemistry may be considered separately from the porous-flow simulation and the heat exchanger simulation.  This section describes how the aquifer geochemistry is simulated.

The model set-up commences with defining the basis species, minerals, and the rate description for the kinetic dissolution of `QuartzLike`:

!listing modules/combined/examples/geochem-porous_flow/geotes_2D/aquifer_geochemistry.i block=UserObjects

The parameters in the rate description are similar to those chosen in [another example](kinetic_quartz_arrhenius.md).

A [SpatialReactionSolver](SpatialReactionSolver/index.md) defines the initial concentrations of the species and how reactants are added to the system

!listing modules/combined/examples/geochem-porous_flow/geotes_2D/aquifer_geochemistry.i block=SpatialReactionSolver

Most lines in this block need further discussion.

- Since this is a `SpatialReactionSolver`, the system described exists at each node in the mesh.  Each node will typically have a different void volume fraction associated to it: nodes in coarsely-meshed regions will be responsible for large volumes of fluid, while nodes in finely-meshed regions, or regions with small porosity, will represent small volumes of fluid.  In the above setup, just *1 liter* of aqueous solution is modelled at each node.  As discussed below, the rates of reactant additions will be adjusted to reflect this.  It is *assumed* that 1 liter of aqueous solution contains 1.0$\,$kg of solvent water and the specified quantities of Na$^{+}$, Cl$^{-}$ and SiO$_{2}$(aq).

- The `free_molality` constraint on SiO$_{2}$(aq) is set according to the equilibrium simulation described above.

- It is assumed that there is 9000$\,$cm$^{3}$ of `QuartzLike` per the 1000$\,$cm$^{3}$ (1 liter) of aqueous solution.  This corresponds to 396.685$\,$mol of `QuartzLike`.  The reason for choosing 9000$\,$cm$^{3}$ of `QuartzLike` is that the aquifer porosity is assumed to be 0.1 and to be calculated using:

\begin{equation}
\phi = \frac{V_{\mathrm{fluid}}}{V_{\mathrm{fluid}} + V_{\mathrm{mineral}}} = \frac{1000}{1000 + 9000} \ .
\end{equation}

- The initial `temperature` of the aquifer is assumed to be 50$^{\circ}$C, and during simulation it is given by the `AuxVariable` called `temperature`.  This `AuxVariable` will be provided by the PorousFlow simulation described below.

- The `source_species_rates` are defined by the `rate_*_per_1l` `AuxVariables` which are discussed further below.

The Mesh and Executioner are provided in the input file so that it may be run alone in a stand-alone fashion, but these are overwritten when coupled with the PorousFlow simulation (below).  The remainder of the input file deals with `AuxVariables` that are used to transfer information to-and-from the PorousFlow simulation.

The PorousFlow simulation provides `temperature`, `pf_rate_H2O`, `pf_rate_Na`, `pf_rate_Cl` and `pf_rate_SiO2` as `AuxVariables`.  The `pf_rate` quantities are rates of changes of fluid-component mass (kg.s$^{-1}$) at each node, but the Geochemistry module expects rates-of-changes of moles (mol.s$^{-1}$) at each node.  Moreover, since this input file considers just 1 liter of aqueous solution at every node, the `nodal_void_volume` is used to convert to mol.s$^{-1}$/(liter of aqueous solution).  For instance:

!listing modules/combined/examples/geochem-porous_flow/geotes_2D/aquifer_geochemistry.i start=[rate_H2O_per_1l_auxk] end=[rate_Na_per_1l]

This input file provides the PorousFlow simulation with updated mass-fractions (after `QuartzLike` dissolution has occured to increase the SiO$_{2}$(aq) concentration, for instance).  These are based on the "transported" mole numbers at each node, which are the mole numbers of the transported species in the original basis (mole numbers of H$_{2}$O, Na$^{+}$, Cl$^{-}$ and free moles of SiO$_{2}$(aq) in this case).  These are recorded into `AuxVariables` using, for example:

!listing modules/combined/examples/geochem-porous_flow/geotes_2D/aquifer_geochemistry.i start=[transported_H2O_auxk] end=[transported_Na]

The mass fraction is computed using, for example

!listing modules/combined/examples/geochem-porous_flow/geotes_2D/aquifer_geochemistry.i start=[transported_mass_auxk] end=[massfrac_Na]


## The PorousFlow simulation

The PorousFlow module is used to perform the transport.  It is assumed the aquifer may be adequately modelled using a 2D mesh.  The injection well is at $(-30, 0)$ and the production well at $(30, 0)$:

!listing modules/combined/examples/geochem-porous_flow/geotes_2D/porous_flow.i block=Mesh

!listing modules/combined/examples/geochem-porous_flow/geotes_2D/production.bh

Recall that the PorousFlow module considers mass fractions, not mole-fractions or molalities, etc.  Denoting the mass-fraction of Na$^{+}$ by `f0`, the mass-fraction of Cl$^{-}$ by `f1`, and the mass-fraction of SiO$_{2}$(aq) by `f2` (the mass-fraction of H$_{2}$O may be worked out by summing mass-fractions to unity), the `Variables` are:

!listing modules/combined/examples/geochem-porous_flow/geotes_2D/porous_flow.i block=Variables

The initial conditions are chosen to reflect the temperature and mole numbers of the aquifer geochemistry input file, and an initial porepressure of 2$\,$MPa is used, corresponding to an approximate depth of 200$\,$m.

Much of the remainder of this input file is typical of PorousFlow simulations.  The points of particular interest here involve the transfer of information to the aquifer geochemistry simulation (above) and the heat-exchanger simulation (below).

### Transfer to the aquifer geochemistry simulation

The PorousFlow input file transfers the rates of changes of each species (kg.s$^{-1}$) at each node to the aquifer geochemistry simulation.  This is achieved through saving these changes from Kernel residual evaluations into AuxKernels, using the `save_component_rate_in`:

!listing modules/combined/examples/geochem-porous_flow/geotes_2D/porous_flow.i block=PorousFlowFullySaturated

### Transfer from the aquifer geochemistry simulation

At the end of each time-step, the aquifer geochemistry simulation provides updated mass-fractions to the PorousFlow simulation:

!listing modules/combined/examples/geochem-porous_flow/geotes_2D/porous_flow.i start=[massfrac_from_geochem] end=[]

### Interfacing with the heat-exchanger simulation

This input file simulates fluid-and-heat injection through the well at $(-30, 0)$ and fluid-and-heat extraction from the well at $(30, 0)$.

The heat injection is modelled using a fixed value of temperature at the `injection_node`:

!listing modules/combined/examples/geochem-porous_flow/geotes_2D/porous_flow.i block=BCs

The `injection_temperature` variable is set by the exchanger simulation (below) via a MultiApp Transfer.  The fluid injection is modelled using a PorousFlowPolyLineSink, such as:

!listing modules/combined/examples/geochem-porous_flow/geotes_2D/porous_flow.i start=[inject_Na] end=[inject_Cl]

In this block:

- The `SumQuantityUO` measures the injected mass, for purposes of checking that the correct amount of fluid has been injected
- The `${injected_rate}` is a constant specified at the top of the input file as 1$\,$kg.s$^{-1}$.m$^{-1}$.
- The combination of `${injected_rate}`, `p_or_t_vals`, `line_length` and `multiplying_var` means that the `variable` (which is `f0`, or the mass-fraction of Na$^{+}$) will be subjected to a source of size `injection_rate_massfrac_Na` at the borehole's position

The four `injection_rate_massfrac_*` variables are set by the exchanger simulation (below) via a MultiApp Transfer.

The fluid and heat production from the other well are also simulated by PorousFlowPolyLineSinks, such as:

!listing modules/combined/examples/geochem-porous_flow/geotes_2D/porous_flow.i start=[produce_Na] end=[produce_Cl]

In this block:

- the `produced_mass_Na` records the amount of Na$^{+}$ produced;
- the `${injected_rate}` is a constant specified at the top of the input file as 1$\,$kg.s$^{-1}$.m$^{-1}$.
- the `mass_fraction_component = 0` instructs PorousFlow to capture only the zeroth mass fraction (Na$^{+}$)

Heat energy is captured using a similar technique (notice the `use_enthalpy` flag):

!listing modules/combined/examples/geochem-porous_flow/geotes_2D/porous_flow.i start=[produce_heat] end=[]

The `produced_mass_*` quantities and the temperature at the production well are provided to the heat-exchanger input file (below).  However, that input file works in mole units, not mass fractions, so a translation is needed:

!listing modules/combined/examples/geochem-porous_flow/geotes_2D/porous_flow.i start=[kg_Na_produced_this_timestep] end=[kg_Cl_produced_this_timestep]

!listing modules/combined/examples/geochem-porous_flow/geotes_2D/porous_flow.i start=[mole_rate_Na_produced] end=[mole_rate_Cl_produced]

!listing modules/combined/examples/geochem-porous_flow/geotes_2D/porous_flow.i start=[moles_Na] end=[moles_Cl]


## The heat-exchanger simulation

This input file uses a [TimeDependentReactionSolver](TimeDependentReactionSolver/index.md) object, but in the absence of data concerning the kinetics of `QuartzLike` in heat-exchangers, this mineral is treated as an equilibrium species (non-kinetic):

!listing modules/combined/examples/geochem-porous_flow/geotes_2D/exchanger.i block=TimeDependentReactionSolver

Notice the following points:

- `QuartzLike` is provided with a very small initial `free_mole_mineral_species`, which means that SiO$_{2}$(aq) will find its initial concentration at equilibrium.
- `mode = 4`, which means "heat-exchanger" mode is used.  This means, at each time-step:

  1. any fluid present in the system is removed;
  2. fluid added by the `source_species` is added;
  3. the system is taken to `cold_temperature`;
  4. any precipitates are recorded and removed from the system (in the case of `QuartzLike`, a little precipitation will occur because `cold_temperature` is less than the aquifer temperature);
  5. the system is heated to `temperature`;
  6. any precipitates are recorded and removed from the system (in the case of `QuartzLike`, no further precipitation will occur because it is more soluble at higher temperatures).

- The `source_species` are set by the `production_rate_*` `AuxVariables`.  These are quantities pumped from the production well in the PorousFlow simulation, and are provided by a MultiApp Transfer from the PorousFlow simulation as described above.

This simulation provides the PorousFlow simulation with the injection temperature (200$^{\circ}$C).  It also provides the mass-fractions of the injected fluid.  This is achieved by recording the "transported" mole numbers:

!listing modules/combined/examples/geochem-porous_flow/geotes_2D/exchanger.i start=[transported_H2O_auxk] end=[transported_Na]

and converting mole numbers to mass fractions:

!listing modules/combined/examples/geochem-porous_flow/geotes_2D/exchanger.i start=[transported_mass_auxk] end=[massfrac_Na]

Quantities of interest are recorded by Postprocessors:

!listing modules/combined/examples/geochem-porous_flow/geotes_2D/exchanger.i block=Postprocessors

## Results

The figures below depict how heat has advected from the hot well to the cold well, as well as how `QuartzLike` dissolution has caused small changes in the porosity of the aquifer.

!media geotes_2D_temperature.png caption=Aquifer temperature after injection of heated aquifer-brine into the system.  The green dots show the injection and production borehole positions.  id=geotes_2D_temperature.fig

!media geotes_2D_porosity.png caption=Aquifer porosity after injection of heated aquifer-brine into the system that causes dissolution of the aquifer quartz.  The green dots show the injection and production borehole positions.  id=geotes_2D_porosity.fig

## Comparison with `QuartzUnlike`

A very similar set of simulations, `aquifer_un_quartz_geochemistry.i` and `exchanger_un_quartz.i` may be found in the same directory.  Instead of `QuartzLike`, these use the `QuartzUnlike` mineral, which precipitates upon *heating* rather than *cooling*.  It is expected that the hot-fluid injection will have little impact upon the aquifer structure, but that lots of `QuartzUnlike` will precipitate in the heat exchanger as it heats the produced reservoir fluid.  Indeed, the following figure shows this occurs:

!media geotes_2D.png caption=Minerals precipitated in the heat exchanger for the QuartzLike and QuartzUnlike cases.  There are no QuartzLike precipitates because it becomes more soluble as it is heated, in contrast to the QuartzUnlike mineral.  id=geotes_2D.fig











# A Geothermal simulation involving 2D flow

This page describes a reactive-transport simulation of a hypothetical geothermal system at the FORGE site.  The transport is handled using the PorousFlow module, while the Geochemistry module is used to simulate the geochemistry.  The simulations are coupled together in an operator-splitting approach using MultiApps.  If the input files appear confusing, the reader could consult the simpler models listed on the [Geochemistry examples page](geochemistry/tests_and_examples/index.md).

The simulation involves pumping cold fluid into a thin, hot subsurface confined aquifer, and recording the resulting geochemical changes.  Before getting to the reactive-transport simulation, however, a lot of exploratory geochemical modelling is needed.

Although the geochemistry module can accept input in the form of various convenient units such as mg/kg, $\mu$g, etc, this page uses mole-based units exclusively.

## Representative water composition

Water composition has been measured at Roosevelt Hot Springs, which is close to the FORGE site, and this may be representative of the type of water found in the FORGE fractured granite.  The other type of water used in this presentation is a low TDS potable water, which is the injectate.  The composition of both waters is shown in [table:forge_waters] (after [!cite](vivek)).  Note that there is no iron composition in either of the waters, so no iron-bearing minerals will be considered in this presentation.

!table id=table:forge_waters caption=Waters compositions used in this presentation.  Concentrations of chemical species are measured in mg/kg.
|  | Water1 | Water3 |
| --- | --- | --- |
|   | Roosevelt | Low TDS |
| Temperature | 60$^{\circ}$C | 20$^{\circ}$C |
| pH | 7.5 | 6.2 |
| Na+ | 2710 | 3.03 |
| K+ | 612 | 1.1 |
| Ca++ | 27 | 3.11 |
| Mg++ | 0.02 | 0.7 |
| SiO2(aq) | 220 | 16.4 |
| Al+++ | 0.1 | 0.1 |
| Cl- | 4935 | 0.5 |
| SO4-- | 53 | 1 |
| HCO3- | 87 | 20 |

## Mineralogy

X-ray diffraction suggests the composition of the FORGE granite is as shown in [table:forge_mineralogy].

!table id=table:forge_mineralogy caption=Mineralogy of the FORGE site as suggested by X-ray diffraction
| Mineral | Weight % |
| --- | --- |
| Plagioclase | 47 |
| K-feldspar | 29 |
| Quartz | 18 |
| Illite | 2 |
| Chlorite | trace |
| Chlorite-Smectite | trace |
| Biotite | trace |
| Hornblende | 1 |
| Augite | trace |
| Titanite | trace |
| Apatite | trace |
| Epidote | trace |
| Calcite | trace |

[!cite](vivek) interpret Plagioclase as a 9:1 mix of Albite and Anorthite, while Stuart Simmons suggests that Biotite could be Phlogopite.  Further correspondance with Stuart Simmons has identified a set of representative minerals to be used in the simulations, and they are shown in [table:model_mineralogy]

!table id=table:model_mineralogy caption=Mineralogy used in the simulations
| Mineral | Weight % |
| --- | --- |
| Albite | 44 |
| Anorthite | 5 |
| K-feldspar | 29 |
| Quartz | 18 |
| Illite | 2 |
| Phlogopite | 2 |
| Anhydrite | trace |
| Calcite | trace |
| Paragonite | trace |
| Chalcedony | trace |
| Kaolinite | trace |
| Clinochl-7A | trace |
| Laumontite | trace |
| Ziosite | trace |

## Water1 at equilibrium

A `geochemistry` input file that finds the equilibrium solution for Water1 is

!listing modules/combined/examples/geochem-porous_flow/forge/water_60degC.i

Note  use of the flag

```
remove_all_extrapolated_secondary_species = true
```

This flag removes the odd secondary species whose equilibrium constants have only been measured for a small range of temperatures.  By default, the `geochemistry` module extrapolates the equilibrium constants for such species, and frequently this means the equilibrium constants are unrealistically high or low (such as $10^{1000}$ or $10^{-1000}$) at high temperatures.  This leads to convergence problems, or the simulation converges to an unrealistic solution.  Therefore, this flag is used for all input files in this presentation.

If minerals are allowed to precipitate from Water1, small amounts of K-feldspar and Quartz form.

## Estimating the in-situ reservoir water

When performing reactive-transport simulations, it is necessary to start with a fluid-filled reservoir.  For most purposes, the fluid composition is not critical, since the injectate will rapidly displace it.  However, it is convenient to fill the initial reservoir with fluid that is at equilibrium with the reservoir rocks.  This makes the interpretation of the reactive-transport simulations easier, for any changes in mineralogy can be attributed to the injectate, and not the in-situ reservoir water.  Therefore, this section attempts to find a water composition that is at equilibrium with the reservoir.

In this section, all the minerals present in [table:model_mineralogy] are used, with the exception of Laumontite and Zoisite.  These two are present only in trace quantities (if at all), but if they exist in the model, the rock mineralogy of [table:model_mineralogy] is unstable, so it is impossible to find an equilibrium.  For instance, Anorthite will decompose into Laumontite.  This suggests that the rock mineralogy is controlled by kinetics, which is only considered in later sections (where Laumontite and Zoisite are included).  It also reveals that the modelling results could be significantly impacted by the choice of minerals.

The following method is used to estimate a water composition that is at equilibrium with the reservoir mineralogy.

1. Water1 is equilibrated at 60$^{\circ}$C, with pH fixed to 7.5, allowing any precipitates to form.  Only Quartz and K-feldspar precipitate, as mentioned in the previous section.
2. The system is closed (at $t=0$), i.e. the pH is no longer fixed.  The small amount of Quartz and K-feldspar precipitates are retained.
3. The temperature is raised to 220$^{\circ}$C (during $0<t\leq 1$), allowing any precipitates to form or dissolve.  Quartz dissolves entirely, K-feldspar precipitate remains, and Calcite and Phlogopite precipitate.  The pH becomes 7.078.  Note the use of `remove_all_extrapolated_secondary_species = true` in the GeochemicalModelDefinition.  If the extrapolated secondary species are retained instead, the results are significantly different (and probably incorrect).
4. The following minerals are added (during $1<t\leq 2$): Albite (16.8mol = 44% by weight), Anorthite (1.8mol = 5% by weight), K-feldspar (10.4mol = 29% by weight), Quartz (30.0mol = 18% by weight), Phlogopite (0.48mol = 2% by weight) and Illite (0.52mol = 2% by weight).  The mol numbers are approximately what has been measured by X-ray diffraction as in [table:model_mineralogy].

!listing modules/combined/examples/geochem-porous_flow/forge/water_60_to_220degC.i

The free moles precipitated are Albite 16.38, Anorthite 1.785, K-feldspar 10.68, Quartz 30.82, Phlogopite 0.52, Paragonite 0.44, Calcite 0.0004, Anhydrite 0.0004, while the rest of the minerals do not precipitate.  Notice:

- The added Illite has dissolved in favor of Paragonite.  This is important in interpreting reactive-transport results because if the reactive-transport's initial mineralogy is given by [table:model_mineralogy], Illite could dissolve regardless of the injectate.
- Calcite is constrained by the initial HCO3- concentration
- Anhydrite by the initial SO4-- concentration

!table id=table:water_60_to_220degC_result caption=Water that is in equilibrium with the mineralogy at 220$^{\circ}$C.  The pH is 6.16.
| Species | Concentration (molal) | Concentration (mg/kg)
| --- | --- | --- |
| Na+ | 0.1001 | 2300 |
| K+ | 0.005949 | 233 |
| Ca++ | 0.01465 | 587 |
| Mg++ | 6.39e-06 | 0.16 |
| SiO2(aq) | 0.00452 | 272 |
| Al+++ | 1.667e-06 | 0.05 |
| Cl- | 0.135 | 4790 |
| SO4-- | 9.995e-05 | 9.6 |
| HCO3- | 0.0009765 | 60 |

## In-situ kinetics

The analysis of the previous section produced a water that is in equilibrium with the rock mineralogy, as well as a stable mineral assemblage.  However:

- The analysis revealed that [table:model_mineralogy] may not be in equilibrium because Illite dissolves.  Perhaps this does not matter greatly as Illite's mass fraction is only about 2%.
- The model did not include Laumontite and Zoisite since they are more stable than Anorthite, which appears in appreciable fractions in the X-ray analysis.

Presumably all these minerals are controlled by [kinetic reactions](theory/index.md), and using kinetics may result in quite different results.  Therefore, this section performs an analysis of the in-situ reservoir kinetics to act as a baseline for the reactive-transport simulations.

In this presentation, kinetic rates are modelled using the formula
\begin{equation}
\label{eqn:rate}
r = MS k \exp\left(\frac{E_{a}}{R(T - 298.15)}\right) \ ,
\end{equation}
where

- $r$ \[mol.s$^{-1}$\] is the reaction rate
- $M$ \[g\] is the mass of the mineral.  This is the number of moles multiplied by the molar weight.
- $S$ \[cm$^{2}$.g$^{-1}$\] is the specific surface area of the mineral
- $k$ \[mol.s$^{-1}$.m$^{-2}$\] is the reaction's intrinsic rate constant
- $E_{a}$ \[J.mol$^{-1}$\] is the reaction's activiation area
- $R = 8.314\ldots\,$J.K$^{-1}$.mol$^{-1}$ is the gas constant
- $T$ \[K\] is the temperature 

[!cite](palandri) list parameters for a wide number of minerals.  In many cases these are based on limited experimental data, so may be subject to large errors.  

!table id=table:kinetic_rates caption=Rates of the mineral species used in this presentation.  $S$ \[cm$^{2}$.g$^{-1}$\] is the specific surface area of the mineral, $k$ \[mol.s$^{-1}$.m$^{-2}$\] is the reaction's intrinsic rate constant and $E_{a}$ \[J.mol$^{-1}$\] is the reaction's activiation area
| Species | Palandri reference | Assumed S | k | E$_{a}$ |
| --- | --- | --- | --- | --- |
| Albite | page8, neutral mechanism | 10 | $10^{-17}$ | 69800 |
| Anhydrite | page43, neutral mechanism | 10 | $10^{-7}$ | 14300 |
| Anorthite | page24, neutral mechanism | 10 | $10^{-13}$ | 17800 |
| Calcite | page42, neutral mechanism | 10 | $10^{-10}$ | 23500 |
| Chalcedony | Assumed same as Quartz  | 10 | $10^{-18}$ | 90100 |
| Clinochl-7A | page40, neutral Chlinochlor-14A | 10 | $10^{-17}$ | 88000 |
| Illite | Assumed same as Phlogopite | 10 | $10^{-17}$ | 29000 |
| K-feldspar | page26, neutral mechanism | 10 | $10^{-17}$ | 38000 |
| Kaolinite | page39, neutral mechanism | 10 | $10^{-18}$ | 22200 |
| Laumontite | No data | 10 | $10^{-15}$ | 17800 |
| Quartz | page18, neutral mechanism f | 10 | $10^{-18}$ | 90100 |
| Paragonite | page38, neutral mechanism | 10 | $10^{-17}$ | 22000 |
| Phlogopite | page38, neutral mechanism | 10 | $10^{-17}$ | 29000 |
| Zoisite | page35, average of others | 10 | $10^{-16}$ | 66100 |

It is useful to analyse the rate of Illite and Anorthite conversion in order to act as a reference to the reactive-transport simulations

- The in-situ reservoir water from the previous section is used
- Minerals with ratios given in [table:model_mineralogy] are introduced
- Trace minerals are assumed to exist at a 0.1% volume fraction.  It is necessary to have some mineral present in the simulation for the rate of precipitation and dissolution, [eqn:rate], depends on the mineral mass: with zero mineral there can be no precipitation of that mineral!
- The quantities of the trace minerals Calcite and Anhydrite are constrained by the initial HCO3- and SO4-- concentration, respectively
- For every litre of in-situ reservoir water, 99 litres of rock mineral is used, representing a porosity of 0.01
- After the minerals are introduced, the system is closed (no further water is allowed to enter the system) and its evolution is followed


The following input file models this situation

!listing modules/combined/examples/geochem-porous_flow/forge/natural_reservoir.i

Results are shown in [natural_reservoir.fig], [natural_reservoir_percentage.fig] and [natural_reservoir_solution.fig]  The figures reveal that:

- The small amount of Calcite dissolves entirely within $10^{-2}$ years
- About 25% of the small amount of Anhydrite dissolves rapidly, but after 100 years it returns to its original volume
- Between 1 and 100 years after the simulation commences, all Anorthite, Chalcedony and Clinochl-7A dissolve
- Between 1 and 100 years after the simulation commences, Zoisite, Illite, Paragony and Quartz increase in volume.  Laumonite increases slightly too, before returning to its original small value.
- The pH drops to around 4.8 during the course of the first 100 years before rebounding to 5.8.
- The solvent water mass roughly halves during the first 100 years, corresponding to small porosity decreases.

The results depend quite crucially on the kinetic rate constants, which are poorly constrained by experiment.  For instance, increasing Laumontite's rate constant by a factor of 10 means that all the water gets absorbed into minerals as they precipitate, resulting in geothermal reservoir with zero porosity.

This concludes the study on the in-situ dynamics of the reservoir.  Attention is now turned to the injectate and, finally, reactive-transport modelling.


!media natural_reservoir_minerals.png caption=Change in mineral volume when the in-situ reservoir water is in contact with the minerals of [table:model_mineralogy].  id=natural_reservoir.fig

!media natural_reservoir_mineral_percentage.png caption=Percentage change in mineral volume when the in-situ reservoir water is in contact with the minerals of [table:model_mineralogy].  id=natural_reservoir_percentage.fig

!media natural_reservoir_solution.png caption=Changes in the water volume and chemistry when the in-situ reservoir water is in contact with the minerals of [table:model_mineralogy].  id=natural_reservoir_solution.fig



## Water3 at 70$^{\circ}$C

The reactive-transport simulation involves injecting Water3 at 70$^{\circ}$C and it is useful to determine the free molalities of species in this water.  To find these:

1. The initial equilibrium is found at 20degC.  This is the temperature at which the bulk composition was measured, and at this temperature most species are supersaturated.  However, since measurements were performed in the absence of free minerals, their precipitation must be retarded in some way, so all minerals are prevented from precipitating in the model
2. The pH constraint is removed and the system is raised to 70degC, which is the injection temperature.  This causes the pH to drop from 6.2 to 6.1, and only Kaolinite and Illite are supersaturated

An input file that performs this simulation is

!listing modules/combined/examples/geochem-porous_flow/forge/water_3.i

Note that in the [TimeDependentReactionSolver](TimeDependentReactionSolver/index.md):

- The pH is initially set to 6.2 by setting the activity of H+ to $6.31\times 10^{-7}$
- All minerals are prevented from precipitating
- The initial temperature is 20$^{\circ}$C
- After the system has equilibrated during the initial setup phase, the activity constraint on H+ is removed at $t=0$
- The temperature after the initial setup phase is set to 70$^{\circ}$C


## Injectate impacts on the reservoir mineralogy

Water3 at 70$^{\circ}$C (from the previous section) is mixed with rock with composition of [table:model_mineralogy] in order to assess the possible changes in the reservoir.  This is not a reactive transport simulation: it is simply flushing Water3 repeatedly through the rock mineral assemblage.  That is, as soon as a mineral dissolves, the aqueous components are swept away and replaced by a new batch of Water3.  Or, as soon as a precipitate forms from Water3, it is retained and a new batch of Water3 is introduced, ready to potentially form more precipitate.  The rate constants of [table:kinetic_rates] are used.

An input file that performs this simulation is

!listing modules/combined/examples/geochem-porous_flow/forge/reservoir_and_water_3.i

The results are shown in [reservoir_and_water_3.fig] and [reservoir_and_water_3_percentage.fig], which should be explored keeping the baseline of [natural_reservoir.fig] and [natural_reservoir_percentage.fig] in mind.

- The cool injectate causes complete dissolution of the small quantities of Anhydrite
- The large quantities of Anorthite and small quantities of Calcite completely dissolve, but this could also be due to natural reservoir processes
- After 100 years, the cool injectate causes gradual and complete dissolution of the small quantities of Zoisite and Laumontite
- After 100 years, the small quantities of Clinochl-7A gradually dissolve, but this could also be due to natural reservoir processes
- After 100 years, large quantities of Albite begin to dissolve
- After 1000 years, quantities of K-feldspar begin to dissolve
- Small quantities of Kaolinite precipitate, but no other minerals precipitate

The key results are:

- Flushing with Water3 at 70$^{\circ}$C prevents any appreciable precipitation
- Hence, the dissolution of Anorthite commencing after about 1 year, and the dissolution of Albite and K-feldspar later on lead to significant loss of mineral, and a consequential increase in porosity or fracture aperture

Once again, the results depend quite crucially on the kinetic rate constants, which are poorly constrained by experiment.


!media reservoir_and_water_3.png caption=Change in mineral volume when Water3 at 70$^{\circ}$C is flushed through the mineral assemblage of [table:model_mineralogy].  id=reservoir_and_water_3.fig

!media reservoir_and_water_3_percentage.png caption=Percentage change in mineral volume when Water3 at 70$^{\circ}$C is flushed through the mineral assemblage of [table:model_mineralogy].  id=reservoir_and_water_3_percentage.fig

## Injection simulation without geochemistry

The simulation involves injecting cold fluid into a thin, hot subsurface confined aquifer.  The PorousFlow module is used to simulate the transport of solutes and temperature.  The model parameters are displayed in [table:pf_params].

!table id=table:pf_params caption=Parameters of the porous-flow simulation
| Quantity | Value |
| --- | --- |
| Aquifer thickness | 1$\,$m |
| Gravity | 0 |
| Reservoir initial temperature | 220$^{\circ}$C |
| Reservoir initial pressure | 20$\,$MPa |
| Reservoir permeability | $10^{-14}\,$m$^{2}$ |
| Reservoir porosity | 0.01 |
| Reservoir thermal conductivity | 2.5$\,$W.m$^{-1}$.K$^{-1}$ |
| Reservoir heat capacity | 2.475$\,$MJ.K$^{-1}$ |
| Injection rate | 0.25$\,$kg.s$^{-1}$ |
| Injectate temperature | 70$^{\circ}$C |
| Production bottomhole pressure | 20$\,$MPa |
| Separation between injection and production wells | 100$\,$m |

PorousFlow operates using mass-fractions and not mole numbers, so the species concentrations in the reservoir [table:water_60_to_220degC_result] and and injectate [table:forge_waters] must be converted, as shown in [table:mass_fractions].

!table id=table:mass_fractions caption=Mass fractions of waters (charge-balance on Cl-)
| Species | In-situ (g/kg) | In-situ (mass frac) | Injectate (Water3) | Injectate (mass frac) |
| --- | --- | --- | --- | --- |
Al+++ | 4.50E-05 | 4.46E-08 | 1.00E-04 | 1.00E-07 |
Ca++ | 5.87E-01 | 5.82E-04 | 3.11E-03 | 3.11E-06 |
Cl- | 4.79E+00 | 4.74E-03 | 7.99E-03 |  7.99E-06 |
H+ | 8.27E-04 | 8.20E-07 | 1.92E-04 | 1.92E-07 |
H2O | 1.00E+03 | 9.92E-01 | 1.00E+03 |  1.00 |
HCO3- | 5.96E-02 | 5.91E-05 | 2.00E-02 |  2.00E-05 |
K+ | 2.33E-01 | 2.31E-04 | 1.10E-03 | 1.10E-06 |
Mg++ | 1.55E-04 | 1.54E-07 | 7.00E-04 | 7.00E-07 |
Na+ |  2.30E+00 | 2.28E-03 | 3.03E-03 | 3.03E-06 |
SO4-- | 9.60E-03 | 9.52E-06 | 9.99E-04 | 9.99E-07 |
SiO2 | 2.72E-01 | 2.69E-04 | 1.64E-02 |  1.64E-05 |

The input file that simulates this situation is (ignore the MultiApps and Transfers for the time being):

!listing modules/combined/examples/geochem-porous_flow/forge/porous_flow.i

Some results concerning the produced fluid are shown in [forge_production_rate.fig], [forge_production_rate_SiO2.fig] and [forge_production_temperature.fig].  These figures demonstrate that:

- apart at the very beginning of the simulation, the production rate equals the injection rate;
- fluid break-through occurs quite rapidly --- certainly in less than a month after injection commences;
- thermal break-through occurs after about 6 months of injection.

!media forge_production_rate.png caption=Production rate.  id=forge_production_rate.fig

!media forge_production_rate_SiO2.png caption=Production fraction of SiO2(aq).  id=forge_production_rate_SiO2.fig

!media forge_production_temperature.png caption=Production temperature.  id=forge_production_temperature.fig

Some contour plots are shown in [forge_pf_SiO2_contour.fig] and [forge_pf_temp_contour.fig].  The former demonstrates the injectate fluid-breakthrough after only 1 week of injecting.

!media forge_pf_SiO2_contour.png caption=Mass-fraction of SiO2(aq) contour after only 1 week of injection.  The green dots show the injection and production wells.  id=forge_pf_SiO2_contour.fig

!media forge_pf_temp_contour.png caption=Temperature contour after 1 year of injection.  The green dots show the injection and production wells.  id=forge_pf_temp_contour.fig




## Reactive transport simulation

The sections above have prepared the ground for a reactive transport simulation.  The geochemistry part of the reactive-transport simulation is:

!listing modules/combined/examples/geochem-porous_flow/forge/aquifer_geochemistry.i

Exploring this input file reveals:

- about half the file is virtually identical to the `natural_reservoir.i` simulation studied above that simulates the in-situ kinetics without the influence of an injectate
- the other half of the input file involves the interface with the `porous_flow.i` simulation.

The PorousFlow simulation (listed in the previous section) controls the flow of aqueous species, and interfaces with the aquifer-geochemistry simulation.

1. The PorousFlow simulation provides the AuxVariables called `pf_rate_H`, `pf_rate_Na`, `pf_rate_K`, etc.  These are the changes of mass of each species at each node.
2. The PorousFlow simulation also provides an AuxVariable `temperature` that specifies the temperature at each node.
3. Since the `pf_rate_*` variables have units kg.s$^{-1}$ but the Geochemistry module expects rates-of-changes of moles, a conversion must take place.  Secondly, the aquifer-geochemistry simulation considers just 1 litre of aqueous solution at every node, so the `nodal_void_volume` (amount of fluid at each node) is used in the conversion:

!listing modules/combined/examples/geochem-porous_flow/forge/aquifer_geochemistry.i start=[rate_H_per_1l_auxk] end=[rate_Na_per_1l_auxk]

4. The `geochemistry` code solves the geochemical system at each node, which depends on the rates of source species (`rate_*_per_1l`) and the kinetics.
5. The mass-fraction of each species at each node is computed from the transported mole numbers, and sent back to the `porous_flow.i` simulation in order for the next step of transport:

!listing modules/combined/examples/geochem-porous_flow/forge/aquifer_geochemistry.i start=[transported_mass_auxk] end=[massfrac_Na_auxk]

Some results are shown in [forge_rt_1year_temperature.fig] to [forge_rt_1year_so4.fig].  The following may be surmised from these figures.

- The regions 50$\,$m from the injection well, and between the two wells experience temperature changes.
- The region with approximately 300$\,$m from the wells is impacted by the injectate fluid chemistry.  The impacts are more obvious for some species than others.
- The porosity decreases in response to the injectate, but it appears that porosity changes in response to cool injectate more than heated injectate.

!media forge_rt_1year_temperature.png caption=Temperature contour after 1 year of injection in the reactive-transport simulation.  The green dots show the injection and production wells and the black ring is the 100$^{\circ}$C contour.  id=forge_rt_1year_temperature.fig

!media forge_rt_1year_porosity.png caption=Porosity contour after 1 year of injection in the reactive-transport simulation.  The green dots show the injection and production wells and the black ring is the 100$^{\circ}$C contour.  id=forge_rt_1year_porosity.fig

!media forge_rt_1year_albite.png caption=Contour of free Albite volume after 1 year of injection in the reactive-transport simulation.  The green dots show the injection and production wells and the black ring is the 100$^{\circ}$C contour.  id=forge_rt_1year_albite.fig

!media forge_rt_1year_pH.png caption=Contour of pH after 1 year of injection in the reactive-transport simulation.  The green dots show the injection and production wells and the black ring is the 100$^{\circ}$C contour.  id=forge_rt_1year_pH.fig

!media forge_rt_1year_sio2.png caption=Contour of SiO2(aq) mass fraction after 1 year of injection in the reactive-transport simulation.  The green dots show the injection and production wells and the black ring is the 100$^{\circ}$C contour.  id=forge_rt_1year_sio2.fig

!media forge_rt_1year_so4.png caption=Contour of SO4-- mass fraction after 1 year of injection in the reactive-transport simulation.  The green dots show the injection and production wells and the black ring is the 100$^{\circ}$C contour.  id=forge_rt_1year_so4.fig


!bibtex bibliography
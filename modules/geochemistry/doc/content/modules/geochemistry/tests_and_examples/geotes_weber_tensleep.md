# A GeoTES experiment involving the Weber-Tensleep formation

This page describes a reactive-transport simulation of a GeoTES system in the Weber-Tensleep formation.  The transport is handled using the PorousFlow module, while the Geochemistry module is used to simulate a heat exchanger and the geochemistry.  The simulations are coupled together in an operator-splitting approach using MultiApps.

In Geological Thermal Energy Storage (GeoTES), hot fluid is pumped into an subsurface confined aquifer, stored there for some time, then pumped back again to produce electricity.  Many different scenarios have been proposed, including different borehole geometries (used to pump the fluid), different pumping strategies and cyclicities (pumping down certain boreholes, up other boreholes, etc), different temperatures, different aquifer properties, etc.  This page only describes the heating phase of a particularly simple set up.

Two wells (boreholes) are used.

1. The cold (production) well, which pumps aquifer water from the aquifer.
2. The hot (injection) well, which pumps hot fluid into the aquifer.

These are connected by a heat-exchanger, which heats the aquifer water produced from (1) and passes it to (2).  This means the fluid pumped through the injection well is, in fact, heated aquifer water, which is important practically from a water-conservation standpoint, and to assess the geochemical aspects of circulating aquifer water through the GeoTES system.

!media geotes_2D_schematic.png caption=Schematic of the simple GeoTES simulation.  id=geotes_2D_schematic.fig

## Observed geochemical composition and aquifer mineralogy

The composition of the Weber formation water has been measured at 25$^{\circ}$C and a typical analysis is shown in [table:weber_analysis].  Some minor points are:

- Since HS$^{-}$ is a redox species, its concentration fixes the oxidation state of the water (it is swapped with the basis species O$_{2}$(aq) in the simulations).
- The species NH$_{3}$ is not a basis species, but may be swapped into the basis to replace NO$_{3}^{-}$ in the simulations.
- The species B is not a basis species: B(OH)$_{3}$ is used instead (with concentration 412$\,$mg.L$^{-1}$).
- The species Si is not a basis species: SiO$_{2}$(aq) is used instead (with concentration 96$\,$mg.L$^{-1}$).

!table id=table:weber_analysis caption=Typical composition of the Weber formation water measured at 25$^{\circ}$C
| Species | Concentration (mg.L$^{-1}$) |
| --- | --- |
| Cl- | 57400 |
| SO4-- | 6030 |
| HCO3- | 3996 |
| HS- | 127 |
| Si | 45 |
| Al+++ | 3.5 |
| Ca++ | 539 |
| Mg++ | 45 |
| Fe++ | 44 |
| K+ | 1910 |
| Na+ | 36500 |
| Sr++ | 14 |
| F- | 6.1 |
| B | 72 |
| Br- | 99 |
| Ba++ | 14 |
| Li+ | 91 |
| NH3 (as N) | 33 |
| pH | 6.46 |

The mineralogy of the Weber-Tensleep aquifer has also been measured and a typical composition is shown in [table:weber_mineralogy].  The volume fractions do not sum to 100% because of small measurement errors.

!table id=table:weber_mineralogy caption=Typical mineralogy of the Weber-Tensleep aquifer
| Mineral | Volume fraction (%) |
| --- | --- |
| Quartz | 80.7 |
| K-feldspar | 8.0 |
| Kaolinite  | 6.6E-05 |
| Siderite   | 2.0 |
| Goethite   | 0 |
| Pyrrhotite | 0.10 |
| Dolomite   | 2.0 |
| Calcite    | 5.0 |
| Fe-chlorite | 1.0 |
| Illite     | 1.0 |
| Chalcedony | 0.11 |
| Anhydrite  | 0.60 |
| Barite     | 4.9E-05 |
| Celestite  | 0 |
| Fluorite   | 0 |
| Albite     | 0 |


## Equilibrium model at 25$^{\circ}$C

The simplest simulation possible is one that finds the molality of each primary and secondary species, given the measured concentrations of [table:weber_analysis] while preventing any mineral precipitation.  To perform this analysis, the concentrations shown in [table:weber_analysis] must be converted to mole numbers, as shown in [table:eqm_25deg].

In [table:eqm_25deg] it is assumed that 1$\,$L of aqueous solution contains exactly 1$\,$kg of solvent water.

!table id=table:eqm_25deg caption=Composition of the model at 25$^{\circ}$C.  The pH is 6.46.
| Species | Measured conc (mg.L$^{-1}$) | Mol weight (g/mol) | Molal (mol/kg(solvent water)) |
| --- | --- | --- | --- |
| Cl- | 57400 | 35.453 | 1.619044933 |
| SO4-- | 6030 | 96.0576 | 0.062774835 |
| HCO3- | 3996 | 61.0171 | 0.065489838 |
| HS- | 127 | 33.0679 | 0.003840583 |
| SiO2(aq) | 96 | 60.0843 | 0.001597755 |
| Al+++ | 3.5 | 26.9815 | 0.000129719 |
| Ca++ | 539 | 40.08 | 0.013448104 |
| Mg++ | 45 | 24.305 | 0.001851471 |
| Fe++ | 44 | 55.847 | 0.000787867 |
| K+ | 1910 | 39.0983 | 0.048851229 |
| Na+ | 36500 | 22.9898 | 1.587660615 |
| Sr++ | 14 | 87.62 | 0.000159781 |
| F- | 6.1 | 18.9984 | 0.00032108 |
| B(OH)3 | 412 | 61.8329 | 0.006663119 |
| Br- | 99 | 79.904 | 0.001238987 |
| Ba++ | 14 | 137.33 | 0.000101944 |
| Li+ | 91 | 6.941 | 0.013110503 |
| NH3 | 33 | 17.034 | 0.001937302 |

The MOOSE model begins by defining all the basis species --- those in [table:eqm_25deg] as well as H2O, H+ (to fix the pH) and O2(aq) (to allow for redox couples, in particular HS- and Fe+++) --- as well as the minerals of interest, which are those in [table:weber_mineralogy] with the exception of Fe-Chlorite since it is not in the database:

!listing modules/combined/examples/geochem-porous_flow/geotes_weber_tensleep/eqm_model_25degC_no_precip.i block=UserObjects

!alert note
None of the input files in this page set `remove_all_extrapolated_secondary_species = true` in the [GeochemicalModelDefinition](GeochemicalModelDefinition.md).  Setting this flag to true removes the odd secondary species whose equilibrium constants have only been measured for a small range of temperatures, since extrapolating the experimental results can lead to unrealistic values that cause convergence issues.  It is simply fortunate that the simulations on this page do not need this flag set.  Generally, this flag should be set true in models involving high temperatures.

A [TimeIndependentReactionSolver](AddTimeIndependentReactionSolverAction.md) defines:

- the swaps mentioned above (so the measured concentrations of NH3 and HS- can be used instead of NO3- and O2(aq))
- the charge-balance species, which is assumed to be Cl-
- the bulk mole numbers of species from [table:eqm_25deg] and the pH (instead of mole numbers, the geochemistry module can accept other units such as mg, if more convenient)
- the temperature
- that no minerals are allowed to precipitate in this exploratory simulation.

!listing modules/combined/examples/geochem-porous_flow/geotes_weber_tensleep/eqm_model_25degC_no_precip.i block=TimeIndependentReactionSolver

The MOOSE output includes overall information:

```
Mass of solvent water = 1kg
Total mass = 1.103kg
Mass of aqueous solution = 1.103kg (without free minerals)
pH = 6.46
pe = -2.959
Ionic strength = 1.634mol/kg(solvent water)
Stoichiometric ionic strength = 1.704mol/kg(solvent water)
Activity of water = 0.9439
Temperature = 25
```

and reveals that many minerals are supersaturated, as expected because this water was sourced from an aquifer containing minerals and then cooled:

```
Minerals:
Illite = 5*H2O - 8*H+ + 3.5*SiO2(aq) + 2.3*Al+++ + 0.25*Mg++ + 0.6*K+;  log10K = 9.796;  SI = 9.792
Kaolinite = 5*H2O - 6*H+ + 2*SiO2(aq) + 2*Al+++;  log10K = 7.434;  SI = 7.731
K-feldspar = 2*H2O - 4*H+ + 3*SiO2(aq) + 1*Al+++ + 1*K+;  log10K = 0.06546;  SI = 7.166
Albite = 2*H2O - 4*H+ + 3*SiO2(aq) + 1*Al+++ + 1*Na+;  log10K = 3.081;  SI = 5.703
Pyrrhotite = -0.125*H2O - 0.7188*H+ + 0.03125*SO4-- + 0.875*Fe++ + 0.9688*HS-;  log10K = -5.557;  SI = 3.52
Barite = 1*SO4-- + 1*Ba++;  log10K = -9.973;  SI = 2.614
Quartz = 1*SiO2(aq);  log10K = -4.01;  SI = 1.376
Chalcedony = 1*SiO2(aq);  log10K = -3.738;  SI = 1.104
Siderite = -1*H+ + 1*HCO3- + 1*Fe++;  log10K = -0.2225;  SI = 0.8189
Dolomite = -2*H+ + 2*HCO3- + 1*Ca++ + 1*Mg++;  log10K = 2.516;  SI = 0.3859
Calcite = -1*H+ + 1*HCO3- + 1*Ca++;  log10K = 1.711;  SI = 0.01748
Fluorite = 1*Ca++ + 2*F-;  log10K = -10.97;  SI = -0.1147
Celestite = 1*SO4-- + 1*Sr++;  log10K = -6.443;  SI = -0.6262
Anhydrite = 1*SO4-- + 1*Ca++;  log10K = -4.274;  SI = -1.118
```

One other useful piece of information that is reported is the bulk composition of H+, which is `0.019675774mol`.

## Geochemical technicalities

At this point, it is worth recognising a couple of apparently strange features of the analyses presented in [table:weber_analysis] and [table:weber_mineralogy].

- Barite is supersaturated.  If it is allowed to precipitate then it will remove almost all of the Ba++ found in solution, seemingly contradicting [table:weber_analysis].  According to [table:weber_mineralogy] Barite is found in only very small concentrations, so mechanisms must be present to prevent its precipitation, such as kinetic laws.
- Both Chalcedony and Quartz are present in [table:weber_mineralogy].  Both are SiO2, but Quartz is more stable, so over time all Chalcedony should disappear.  Kinetic rates may be responsible for these observations.

These complexities (and other more subtle ones) are completely ignored in the following, since the purpose of this presentation is to describe how to use the `geochemistry` module and not to get stuck on technical details of the geochemistry, even if they turn out to be important in real life.

## A geochemical model of the Weber-Tensleep formation at 92$^{\circ}$C

The temperature of the Weber-Tensleep aquifer is approximately 92$^{\circ}$C, and its porosity is approximately 0.1.  To build a geochemical model of the formation, the following process is used:

1. The water with composition described by [table:eqm_25deg] is equilibrated at 25$^{\circ}$C while preventing any precipitation of minerals (as in the previous section)
2. The pH constraint is removed (so no more H+ is allowed to enter the system)
3. The system is brought to 92$^{\circ}$C, allowing precipitation
4. The system is assumed to have volume 1$\,$L, or 1000$\,$cm$^{3}$, so is brought into contact with 9000$\,$cm$^{3}$ of minerals with composition shown [table:weber_mineralogy].  The porosity is therefore close to $1000/(1000 + 9000) = 0.1$ (it is not exactly 0.1 due to the minor amounts of precipitation at step 4).
5. The resulting system is brought to equilibrium, allowing minerals to precipitate or dissolve if required, and assuming that no minerals are governed by kinetic laws

It is convenient to use the equilibrium model of the previous section to model steps 1 and 2.  The bulk composition of H+ (`0.019675774mol`) is used, and the source mineral species are shown in [table:model_mineralogy].  Chalcedony is not included because it dissolves in favor of Quartz, and Fe-Chlorite is not included because it does not appear in the database.  To ensure the final porosity is close to 0.1,
\begin{equation}
\mathrm{Model\ volume\ (cm}^{3}\mathrm{)} = 9000 \times \mathrm{Model\ vol\ (\%)} / 100 \ .
\end{equation}

!table id=table:model_mineralogy caption=Mineralogy used in the model, where the porosity is assumed to be 0.1
| Mineral | Measured vol (%) | Model vol (%) | Model vol (cm$^{3}$) | Molar volume (cm$^{3}$.mol$^{-1}$) | Moles |
| --- | --- | --- | --- | --- | --- |
| Siderite | 2 | 2 | 180 | 28.63 | 6.287111422 |
| Pyrrhotite | 0.1 | 0.1 | 9 | 17.62 | 0.510783201 |
| Dolomite | 2 | 2 | 180 | 64.365 | 2.796550921 |
| Illite | 1 | 1 | 90 | 138.94 | 0.647761624 |
| Chalcedony | 0.11 | 0 | 0 | 22.688 | 0 |
| Anhydrite | 0.6 | 0.6 | 54 | 45.94 | 1.175446234 |
| Calcite | 5 | 5 | 450 | 36.934 | 12.1838956 |
| Quartz | 80.7 | 81.299885 | 7316.98965 | 22.688 | 322.504833 |
| K-feldspar | 8 | 8 | 720 | 108.87 | 6.613392119 |
| Kaolinite | 6.60E-05 | 6.60E-05 | 0.00594 | 99.52 | 5.96865E-05 |
| Fe-Chlorite | 1 | 0 | 0 | 1 | 0 |
| Barite | 4.90E-05 | 4.90E-05 | 0.00441 | 52.1 | 8.46449E-05 |
| Goethite | 0 | 0 | 0 | 20.82 | 0 |
| Celestite | 0 | 0 | 0 | 46.25 | 0 |
| Fluorite | 0 | 0 | 0 | 24.54 | 0 |
| Albite | 0 | 0 | 0 | 100.07 | 0 |

The MOOSE input file uses a [TimeDependentReactionSolver](AddTimeDependentReactionSolverAction.md) to raise the temperature and add the source minerals:

- the first part of this input-file block is identical to the model above, save for fixing the bulk composition of H+ instead of fixing the pH
- the second part adds the source minerals at rate given by [table:model_mineralogy] at a temperature of 95$^{\circ}$C so that the final temperature is around 92$^{\circ}$C as desired.

!listing modules/combined/examples/geochem-porous_flow/geotes_weber_tensleep/eqm_model_25_to_92degC.i block=TimeDependentReactionSolver


The MOOSE output includes the summary:

```
Mass of solvent water = 0.9979kg
Total mass = 25.23kg
Mass of aqueous solution = 1.096kg (without free minerals)
pH = 6.899
pe = -3.376
Ionic strength = 1.576mol/kg(solvent water)
Stoichiometric ionic strength = 1.432mol/kg(solvent water)
Activity of water = 0.9532
Temperature = 92.06
```

The MOOSE output also includes the composition in the species basis that consists of the minerals and free species as shown in [table:model_bulk].

!table id=table:model_bulk caption=Composition of the model at 92$^{\circ}$C expressed in the basis that includes minerals and free species.  The mass of solvent water is 0.99778351$\,$kg.
| Species | Bulk composition (moles) | Free |
| --- | --- | --- |
Quartz | 322.4086 | 322.41188 moles
Calcite | 12.111108 | 12.111218 moles
K-feldspar | 6.8269499 | 6.8276215 moles
Siderite | 6.2844304 | 6.2829261 moles
Dolomite | 2.8670301 | 2.8672731 moles
Anhydrite | 1.1912027 | 1.1684061 moles
Pyrrhotite | 0.51474767 | 0.51549555 moles
Illite | 0.3732507 | 0.36893829 moles
Kaolinite | 0.20903322 | 0.21365601 moles
Barite | 0.0001865889 | 0.0001853394 moles
Na+ | 1.5876606 | 1.4840437 molal
Cl- | 1.5059455 | 1.4321212 molal
SO4-- | 0.046792579 | 0.031819079 molal
Li+ | 0.013110503 | 0.012928063 molal
B(OH)3 | 0.006663119 | 0.0014134967 molal
Br- | 0.001238987 | 0.0012417393 molal
F- | 0.00032108 | 0.00023740743 molal
Sr++ | 0.000159781 | 0.00013115377 molal
NH3 | 0.001937302 | 3.1632078e-07 molal

It is interesting to compare the concentration of species in the model when the aqueous solution is removed from the mineral assemblage.  This information is also outputted by the `geochemistry` module and is shown in [table:model_transported_bulk]:

- The concentration of Cl- is impacted by charge-neutrality
- The concentration of HCO3-, Al+++, K+ and Ba++ are all much less in the model compared with the observation due to mineral precipitation
- The concentration of species not impacted by minerals is identical (Na+, Sr++, F-, B, Br-, Li+, NH3)
- The model's concentration of the remaining species are similar to the observations

!table id=table:model_transported_bulk caption=Composition at 92$^{\circ}$C after removing minerals in comparison with the original measurements.
| Species | Measured conc (molal) | Model conc (molal) |
| --- | --- | --- | --- |
| Cl- | 1.619044933 | 1.5059455 |
| SO4-- | 0.062774835 | 0.068842508 |
| HCO3- | 0.065489838 | 0.00090808324 |
| HS-  | 0.003840583 | 0.0029683017 |
| SiO2(aq) | 0.001597755 | 0.00055318842 |
| Al+++ | 0.000129719 | 1.3497527e-06 |
| Ca++ | 0.013448104 | 0.022443348 |
| Mg++ | 0.001851471 | 0.00083515093 |
| Fe++ | 0.000787867 | 0.00084984977 |
| K+  | 0.048851229 | 0.0019158317 |
| Na+ | 1.587660615 | 1.5876606 |
| Sr++ | 0.000159781 | 0.000159781 |
| F-  | 0.00032108 | 0.00032108 |
| B(OH)3 | 0.006663119 | 0.006663119 |
| Br- | 0.001238987 | 0.001238987 |
| Ba++ | 0.000101944 | 1.2495026e-06 |
| Li+ | 0.013110503 | 0.013110503 |
| NH3 | 0.001937302 | 0.001937302 |

The modelling also reveals that Goethite, Albite and Flourite are all supersaturated, so will probably precipitate if they are in equilibrium with the aqueous solution, apparently contradicting [table:weber_mineralogy].

```
Goethite = 3.255*H2O - 0.129*Pyrrhotite - 3.345*Kaolinite + 0.8548*Calcite + 0.129*Anhydrite - 0.9839*Dolomite - 2.361*K-feldspar + 3.935*Illite + 1.113*Siderite;  log10K = -1.607;  SI = 1.539
Albite = 0.4*H2O + 0.5*SO4-- - 1.2*Kaolinite + 2*Quartz + 1*Calcite - 0.5*Anhydrite - 0.5*Dolomite - 1.2*K-feldspar + 2*Illite + 1*Na+;  log10K = -2.122;  SI = 0.8307
Fluorite = -1*SO4-- + 1*Anhydrite + 2*F-;  log10K = -5.468;  SI = 0.2909
Chalcedony = 1*Quartz;  log10K = 0.2214;  SI = -0.2214
Celestite = 1*SO4-- + 1*Sr++;  log10K = -6.861;  SI = -0.4025
```

The total mineral volume is reported to be 9005.244392233$\,$cm$^{3}$.  To ensure the porosity is exactly 0.1, the free amount of Quartz is reduced by $5.244392233\,$cm$^{3} = 0.23115269\,$mol in the reactive transport simulations, leading to a bulk composition of $322.177447\,$mol.

This completes the definition of the geochemical system used in this presentation.  In summary:

- the bulk composition is determined by [table:model_bulk] in the basis of that table, amending Quartz to have a bulk composition of $322.177447\,$mol (the geochemistry module can use other units such as g, mg, etc, but this page uses mole-based units exclusively);
- all minerals in the basis of [table:model_bulk] are assumed to be at equilibrium with the aqueous system (the `geochemistry` module can easily handle kinetic reactions --- a simple GeoTES example may be found [here](geotes_2D.md) --- but no kinetics are used in this presentation);
- the minerals Goethite, Albite, and Flourite are assumed to never precipitate


## Scaling and heating the aqueous solution to 160$^{\circ}$C

The geochemical system just defined may be heated to 160$^{\circ}$C in order to explore potential scaling in a heat exchanger.  The following method is used:

1. The aqueous solution is removed from the aquifer, by removing all free minerals are removed from the model just defined
2. The aqueous solution is slowly heated from 92$^{\circ}$C to 160$^{\circ}$C, immediately removing any precipitates as soon as they form

This technique is studying the "worst case" scenario for scaling in a heat exchanger, for it is possible that a precipitate forms only to dissolve upon further heating, but the technique used removes the precipitate and prevents its dissolution.

The [TimeDependentReactionSolver](AddTimeDependentReactionSolverAction.md) defines:

- the swaps needed to bring the minerals into the basis, so it is as described in [table:model_bulk];
- the bulk composition of the resulting basis (from [table:model_bulk], amending Quartz to have a bulk composition of $322.177447\,$mol, recall that the geochemistry module can use other units such as grams or mg if convenient, but this page uses mole-based units exclusively);
- that the Fluorite, Albite and Goethite minerals will not precipitate;
- the initial temperature;
- that `mode = 1` is used, which means all precipitates are removed at the start of each time step;
- the temperature is controlled by the AuxVariable called `temp_controller`.

!listing modules/combined/examples/geochem-porous_flow/geotes_weber_tensleep/scaling.i block=TimeDependentReactionSolver

The Executioner gives meaning to time:

!listing modules/combined/examples/geochem-porous_flow/geotes_weber_tensleep/scaling.i block=Executioner

The `temp_controller` is a simple `FunctionAux`:

!listing modules/combined/examples/geochem-porous_flow/geotes_weber_tensleep/scaling.i start=[temp_controller_auxk] end=[Anhydrite_mol_auxk]

and the moles dumped to the heat exchanger (scaling) are recorded by a sequence of [GeochemistryQuantityAux](GeochemistryQuantityAux.md):

!listing modules/combined/examples/geochem-porous_flow/geotes_weber_tensleep/scaling.i start=[Anhydrite_mol_auxk] end=[Dolomite_mol_auxk]

Recording the results into Postprocessors yields the results shown in [geotes_weber_tensleep_scaling.fig].  It is interesting to compare this with Nic Spycher's model of the Weber-Tensleep formation which predicts:

- around 2$\,$kg/m$^{3}$(formation water) of Anhydrite, or around 0.7$\,$cm$^{3}$/L(formation water), which is quite similar to the current model;
- around 0.03$\,$cm$^{3}$/L(formation water) of Dolomite, 0.01$\,$cm$^{3}$/L(formation water) of Calcite, 0.0025$\,$cm$^{3}$/L(formation water) of Siderite, while the current model predicts none of these.  This could be due to a combination of a different activity model, a different geochemical model (the pH is lower), and a different definition of "scaling".

!media geotes_weber_tensleep_scaling.png caption=Degree of scaling precipitate expected in the heat exchanger according to the model of formation water.  id=geotes_weber_tensleep_scaling.fig

## Spatial aquifer geochemistry input file

Having created the geochemical model at 92$^{\circ}$C, it is easy to create a version that includes spatial dependence.  Although the input file may be run by itself, no interesting phenomena will be observed since the temperature will be fiexed and source-species rates will be zero.  When coupled with a `porous_flow` input file (described below) the temperature and source-species rates will be non-trivial.  The input file begins with a [GeochemicalModelDefinition](GeochemicalModelDefinition.md):

!listing modules/combined/examples/geochem-porous_flow/geotes_weber_tensleep/aquifer_geochemistry.i block=UserObjects

The [SpatialReactionSolver](SpatialReactionSolver/index.md) defines:

- the swaps needed to bring the basis to the state defined in [table:model_bulk]
- the initial composition of [table:model_bulk] (amending Quartz to have a bulk composition of $322.177447\,$mol).  Please note the key concept that each finite element node considers just 1 litre of aqueous solution
- the `initial_temperature`
- that the temperature at each finite-element node will be controlled by the `temperature` AuxVariable (which will be provided by the parent `porous_flow` simulation)
- the source species, and their rates per 1 litre of aqueous solution
- that various AuxVariables are not needed in this simulation

!listing modules/combined/examples/geochem-porous_flow/geotes_weber_tensleep/aquifer_geochemistry.i block=SpatialReactionSolver

The remainder of the input file is mostly concerned with translating between `porous-flow` information (mass fractions and source rates at each node) and geochemistry information (moles in each litre of fluid).  This block translates the `porous_flow` rate, which is the rate of change of a species mass at each node (in kg.s$^{-1}$) into a rate of change per 1 litre of aqueous solution at each node:

!listing modules/combined/examples/geochem-porous_flow/geotes_weber_tensleep/aquifer_geochemistry.i start=[rate_H_per_1l_auxk] end=[rate_Cl_per_1l_auxk]

Here 1.0079 is the molar mass (g.mol$^{-1}$) of the species H$^{+}$, and `nodal_void_volume` is the volume of aqueous solution held at each node.  This block translates the `geochemistry` moles of transported H$^{+}$ into a mass fraction of H$^{+}$:

!listing modules/combined/examples/geochem-porous_flow/geotes_weber_tensleep/aquifer_geochemistry.i start=[massfrac_H_auxk] end=[massfrac_Cl_auxk]

using the `transported_mass`, which is

!listing modules/combined/examples/geochem-porous_flow/geotes_weber_tensleep/aquifer_geochemistry.i start=[transported_mass_auxk] end=[massfrac_H_auxk]

The porosity is calculated using the free mineral volumes:

!listing modules/combined/examples/geochem-porous_flow/geotes_weber_tensleep/aquifer_geochemistry.i start=[porosity_auxk] end=[nodal_void_volume_auxk]



## Injection, production and porous flow

The Weber-Tensleep aquifer is around 200$\,$m thick, and injecting and producing over its entire thickness may lead to unacceptably low efficiencies as the buoyant hot water rises to the top of the aquifer, never to be recovered.  For the purposes of this example, assume there is a 10$\,$m thick horizontal aquifer, bounded above and below by caps.  The physical properties are listed in [table:aquifer_physical].

!table id=table:aquifer_physical caption=Physical properties of the aquifer and caps
| Property | Quantity |
| --- | --- |
| Aquifer thickness | 10$\,$m |
| Aquifer depth | 3000$\,$m |
| Aquifer initial porepressure | 30$\,$MPa |
| Aquifer initial temperature | 92$^{\circ}$C |
| Aquifer horizontal permeability | $1.7\times 10^{-15}\,$m$^{2}$ |
| Aquifer vertical permeability | $4.1\times 10^{-16}\,$m$^{2}$ |
| Aquifer porosity | 0.1 | 
| Aquifer thermal conductivity | 1.3$\,$W.m$^{-1}$.K$^{-1}$ |
| Cap thickness | 20$\,$m |
| Cap isotropic permeability | $10^{-18}\,$m$^{2}$ |
| Cap porosity | 0.01 |
| Cap thermal conductivity | 1.3$\,$W.m$^{-1}$.K$^{-1}$ |
| Geothermal gradient | 0 |

As shown in [geotes_2D_schematic.fig], the well geometry consists of a single vertical well injecting over the entire aquifer thickness and a single vertical well producing over the entire aquifer thickness.  Only the injection phase of the GeoTES system is explored here, with parameters listed in [table:injection].  The injection and production rates are chosen so that the porepressure remains positive at the production well given the relatively low aquifer permeability.

!table id=table:injection caption=Injection parameters of the GeoTES system
| Property | Quantity |
| --- | --- |
| Injection and production rate | 0.2$\,$kg.s$^{-1}$ |
| Injection temperature | 160$^{\circ}$C |
| Injection fluid | Heated production water: all precipitates removed
| Injection time | 90$\,$days |
| Distance between wells | 50$\,$m |

The injection-production simulation may be performed using the `porous_flow` module.  Because this presentation is focussing on `geochemistry`, only the highlights of the `porous_flow` input file are mentioned.

The variables are the mass-fractions of each transported species (which are those in the original geochemical basis before any swaps), called `f_H`, `f_Cl`, `f_SO4`, etc, with initial condition defined by [table:model_transported_bulk] and converted into mass-fractions as shown in [table:initial_composition].

!table id=table:initial_composition caption=Composition at 92$^{\circ}$C of transported species (ie, not including minerals) expressed in the original basis.
| Species | Moles | Molar weight (g.mol$^{-1}$) | Mass (g) | Mass fraction |
| --- | --- | --- | --- | --- |
| H2O | 55.38 | 18.01801802 | 997.8378378 | 0.910314278 |
| H+ | -0.003232 | 1.0079 | -0.003257533 | -2.9718E-06 |
| Cl- | 1.506 | 35.453 | 53.392218 | 0.048709015 |
| SO4-- | 0.06887 | 96.0576 | 6.615486912 | 0.006035221 |
| HCO3- | 0.0009061 | 61.0171 | 0.055287594 | 5.04381E-05 |
| SiO2(aq) | 0.0005525 | 60.0843 | 0.033196576 | 3.02848E-05 |
| Al+++ | 1.35E-06 | 26.9815 | 3.6479E-05 | 3.32793E-08 |
| Ca++ | 0.02247 | 40.08 | 0.9005976 | 0.000821603 |
| Mg++ | 0.0008366 | 24.305 | 0.020333563 | 1.855E-05 |
| Fe++ | 0.0008498 | 55.847 | 0.047458781 | 4.3296E-05 |
| K+ | 0.001913 | 39.0983 | 0.074795048 | 6.82345E-05 |
| Na+ | 1.588 | 22.9898 | 36.5078024 | 0.033305586 |
| Sr++ | 0.0001598 | 87.62 | 0.014001676 | 1.27735E-05 |
| F- | 0.0003211 | 18.9984 | 0.006100386 | 5.5653E-06 |
| B(OH)3 | 0.006663 | 61.8329 | 0.411992613 | 0.000375855 |
| Br- | 0.001239 | 79.904 | 0.099001056 | 9.03174E-05 |
| Ba++ | 1.25E-06 | 137.33 | 0.000171388 | 1.56355E-07 |
| Li+ | 0.01311 | 6.941 | 0.09099651 | 8.30149E-05 |
| NO3- | 0.001937 | 62.0049 | 0.120103491 | 0.000109569 |
| O2(aq) | -0.002426 | 31.9988 | -0.077629089 | -7.082E-05 |

To interface with the child `geochemistry` simulation (detailed above), the rates of change of each transported species must be recorded.  This is performed via the `save_component_rate_in` feature of the PorousFlowFullySaturated Action:

!listing modules/combined/examples/geochem-porous_flow/geotes_weber_tensleep/porous_flow.i start=[time_deriv_H] end=[time_deriv_Cl]

The interface with the child `geochemistry` simulation is created using a MultiApp and Transfers, which

- provide the source-species rates to the `geochemistry` simulation
- update the mass fractions of each transported species using the results of the `geochemistry` simulation

!listing modules/combined/examples/geochem-porous_flow/geotes_weber_tensleep/porous_flow.i block=MultiApps

!listing modules/combined/examples/geochem-porous_flow/geotes_weber_tensleep/porous_flow.i block=Transfers


The injection and production is performed by PorousFlowPolyLinkSinks, for instance

!listing modules/combined/examples/geochem-porous_flow/geotes_weber_tensleep/porous_flow.i start=[inject_H] end=[inject_Cl]

In this block, the `injection_rate_massfrac_H` is set to the initial mass fraction, representing pumping unadulterated reservoir water into the system

!listing modules/combined/examples/geochem-porous_flow/geotes_weber_tensleep/porous_flow.i start=[injection_rate_massfrac_H] end=[injection_rate_massfrac_Cl]

but will eventually be sourced from the parent simulation `exchanger.i` (see below).  The production DiracKernels record the produced mass of each species into Postprocessors for interface with `exchanger.i`:

!listing modules/combined/examples/geochem-porous_flow/geotes_weber_tensleep/porous_flow.i start=[produce_H] end=[produce_Cl]

and this is converted to a mole rate using Functions such as:

!listing modules/combined/examples/geochem-porous_flow/geotes_weber_tensleep/porous_flow.i start=[moles_H] end=[moles_Cl]

## The heat exchanger

The heat exchanger simulation accepts produced water from the `porous_flow` simulation, heats it to 160$^{\circ}$C allowing precipitates to form and removing them, then injects the remaining fluid back to the `porous_flow` simulation.  Its core is the [TimeDependentReactionSolver](AddTimeDependentReactionSolverAction.md):

- the first lines are similar to previous input files, but notice that because `mode = 4` ("exchanger" mode) all the initial fluid is removed before the exchanger starts precipitating fluid from the `porous_flow` simulation
- the output temperature is controlled by the `ramp_temperature` AuxVariable, which ramps to 160$^{\circ}$C over approximately 1 day to allow the `aquifer_geochemistry` simulation to easily converge
- the `source_species` rates are provided by the `porous_flow` simulation

!listing modules/combined/examples/geochem-porous_flow/geotes_weber_tensleep/exchanger.i block=TimeDependentReactionSolver

The `source_species_rates` are provided by the `porous_flow` simulation using a Transfer:

!listing modules/combined/examples/geochem-porous_flow/geotes_weber_tensleep/exchanger.i start=[production_H] end=[production_Cl]

The composition of fluid injected from the exchanger to the `porous_flow` simulation depends on the transported composition:

!listing modules/combined/examples/geochem-porous_flow/geotes_weber_tensleep/exchanger.i start=[transported_H_auxk] end=[transported_Cl_auxk]

which is converted to a mass fraction:

!listing modules/combined/examples/geochem-porous_flow/geotes_weber_tensleep/exchanger.i start=[massfrac_H_auxk] end=[massfrac_Cl_auxk]

before passing to the `porous_flow` simulation:

!listing modules/combined/examples/geochem-porous_flow/geotes_weber_tensleep/exchanger.i start=[injection_H] end=[injection_Cl]

A similar Transfer is used for the temperature of the injected fluid.

The amount of precipitate of each mineral is recorded using a [GeochemstryQuantityAux](GeochemistryQuantityAux.md):

!listing modules/combined/examples/geochem-porous_flow/geotes_weber_tensleep/exchanger.i start=[dumped_Siderite_auxk] end=[dumped_Pyrrhotite_auxk]

## Writing the input files

The input files for the aqueous geochemistry simulation, the porous flow simulation and the heat exchanger simulation are all quite lengthy and repetitive, simply because of the large number of species involved.  To avoid typos, a python script is provided to write these input files:

!listing modules/combined/examples/geochem-porous_flow/geotes_weber_tensleep/create_input_files.py

## Results: precipitates in the heat exchanger

Precipitates form in the heat exchanger as the formation water is heated from the production temperature to 160$^{\circ}$C.  Since the production temperature is more-or-less constant around 92$^{\circ}$C, because the simulation is not run long enough for significant thermal-breakthrough to occur, the precipitation rate is more-or-less constant.  The results are shown in [geotes_weber_tensleep_exchanger.fig].  The results are very similar to the case in which a parcel of formation water is heated --- see [geotes_weber_tensleep_scaling.fig] --- with the greatest problem being Anhydrite.  Note that this simulation heats the formation water, removing any precipitates formed and preventing any subsequent dissolution if any may occur, so it is a "worst-case" scenario.

!media geotes_weber_tensleep_exchanger.png caption=Degree of scaling precipitate expected in the heat exchanger according to the full exchanger-porous_flow-aquifer_geochemistry model.  id=geotes_weber_tensleep_exchanger.fig

## Results: mineralogy change around the injection well

Minerals precipitate and dissolve around the injection well as shown in [geotes_weber_tensleep_mineralogy_abs.fig] and [geotes_weber_tensleep_mineralogy_rel.fig].  By volume, Illite and Kaolinite undergo most dissolution: indeed, 100% of these minerals are dissolved.  In contrast, both K-feldspar and Quartz precipitate.  Because the volume of dissolution exceeds the precipitated volume, the porosity increases marginally from its original value of 0.1.

!media geotes_weber_tensleep_mineralogy_abs.png caption=Absolute change of mineral volume around the injection well.  id=geotes_weber_tensleep_mineralogy_abs.fig

!media geotes_weber_tensleep_mineralogy_rel.png caption=Relative change of mineral volume around the injection well.  id=geotes_weber_tensleep_mineralogy_rel.fig

## Results: 3D contours

[geotes_wt_3D.fig] shows the results after 90 days of simulation.  The input files producing these results employed a fine mesh and set `remove_all_extrapolated_secondary_species = true` in the [GeochemicalModelDefinition](GeochemicalModelDefinition.md) to ensure convergence.

!media geotes_weber_tensleep_3D.png caption=Temperature, porosity, pH and free volume of Quartz after 90 days of injection in the 3D coupled model.  id=geotes_wt_3D.fig

## Efficiencies

A [related page](compute_efficiencies.md) explores the memory consumption, the impact of linear solver choices and the compute-time scaling with the number of processors of this problem.











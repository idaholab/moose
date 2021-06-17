# Simulation to assess natural changes in the reservoir.  Recall that water_60_to_220degC has provided a stable mineral assemblage that is in agreement with XRD observations, and a water at equilibrium with that assemblage.  However, Stuart Simmons suggested including Laumontite and Zoisite into the simulation, and they were not included in water_60_to_220degC since they are more stable than Anorthite, so Anorthite completely dissolves when equilibrium is assumed.  Here, all minerals suggested by Stuart Simmons are added to the system and kinetics are used to determine the time scales of the mineral changes.  The initial water composition is the reservoir water from water_60_to_220degC.
# The initial mole numbers of the kinetic species are chosen to be such that:
# - the mass fractions are: Albite 0.44; Anorthite 0.05; K-feldspar 0.29; Quartz 0.18, Phlgoptite 0.02 and Illite 0.02 with trace amounts of the remaining minerals.  These are similar to that measured in bulk X-ray diffraction results of 10 samples from well 58-32, assuming that "plagioclase feldspar" consists of Albite and Anorthite in the ratio 9:1, and that Biotite is Phlogoptite.  The trace amounts of each mineral are necessary because of the way the kinetics works: precipitation rate is proportional to mineral-species mass, so without any mass, no precipitation is possible.  Precisely:
# - it is assumed that water_60_to_220degC consists of 1 litre of water (there is 1kg of solvent water) and that the porosity is 0.01, so the amount of rock should be 99000cm^3
# - the cm^3 of the trace minerals Calcite and Anhydrite is exactly given by water_60_to_220degC (0.016 and 0.018 respectively)
# - see initial_kinetic_moles.xlsx for the remaining mole numbers
# The results depend on the kinetic rates used and these are recognised to be poorly constrained by experiment
[UserObjects]
  [rate_Albite]
    type = GeochemistryKineticRate
    kinetic_species_name = Albite
    intrinsic_rate_constant = 1E-17
    multiply_by_mass = true
    area_quantity = 10
    activation_energy = 69.8E3
    one_over_T0 = 0.003354
  []
  [rate_Anhydrite]
    type = GeochemistryKineticRate
    kinetic_species_name = Anhydrite
    intrinsic_rate_constant = 1.0E-7
    multiply_by_mass = true
    area_quantity = 10
    activation_energy = 14.3E3
    one_over_T0 = 0.003354
  []
  [rate_Anorthite]
    type = GeochemistryKineticRate
    kinetic_species_name = Anorthite
    intrinsic_rate_constant = 1.0E-13
    multiply_by_mass = true
    area_quantity = 10
    activation_energy = 17.8E3
    one_over_T0 = 0.003354
  []
  [rate_Calcite]
    type = GeochemistryKineticRate
    kinetic_species_name = Calcite
    intrinsic_rate_constant = 1.0E-10
    multiply_by_mass = true
    area_quantity = 10
    activation_energy = 23.5E3
    one_over_T0 = 0.003354
  []
  [rate_Chalcedony]
    type = GeochemistryKineticRate
    kinetic_species_name = Chalcedony
    intrinsic_rate_constant = 1.0E-18
    multiply_by_mass = true
    area_quantity = 10
    activation_energy = 90.1E3
    one_over_T0 = 0.003354
  []
  [rate_Clinochl-7A]
    type = GeochemistryKineticRate
    kinetic_species_name = Clinochl-7A
    intrinsic_rate_constant = 1.0E-17
    multiply_by_mass = true
    area_quantity = 10
    activation_energy = 88.0E3
    one_over_T0 = 0.003354
  []
  [rate_Illite]
    type = GeochemistryKineticRate
    kinetic_species_name = Illite
    intrinsic_rate_constant = 1E-17
    multiply_by_mass = true
    area_quantity = 10
    activation_energy = 29E3
    one_over_T0 = 0.003354
  []
  [rate_K-feldspar]
    type = GeochemistryKineticRate
    kinetic_species_name = K-feldspar
    intrinsic_rate_constant = 1E-17
    multiply_by_mass = true
    area_quantity = 10
    activation_energy = 38E3
    one_over_T0 = 0.003354
  []
  [rate_Kaolinite]
    type = GeochemistryKineticRate
    kinetic_species_name = Kaolinite
    intrinsic_rate_constant = 1E-18
    multiply_by_mass = true
    area_quantity = 10
    activation_energy = 22.2E3
    one_over_T0 = 0.003354
  []
  [rate_Quartz]
    type = GeochemistryKineticRate
    kinetic_species_name = Quartz
    intrinsic_rate_constant = 1E-18
    multiply_by_mass = true
    area_quantity = 10
    activation_energy = 90.1E3
    one_over_T0 = 0.003354
  []
  [rate_Paragonite]
    type = GeochemistryKineticRate
    kinetic_species_name = Paragonite
    intrinsic_rate_constant = 1E-17
    multiply_by_mass = true
    area_quantity = 10
    activation_energy = 22E3
    one_over_T0 = 0.003354
  []
  [rate_Phlogopite]
    type = GeochemistryKineticRate
    kinetic_species_name = Phlogopite
    intrinsic_rate_constant = 1E-17
    multiply_by_mass = true
    area_quantity = 10
    activation_energy = 22E3
    one_over_T0 = 0.003354
  []
  [rate_Laumontite]
    type = GeochemistryKineticRate
    kinetic_species_name = Laumontite
    intrinsic_rate_constant = 1.0E-15
    multiply_by_mass = true
    area_quantity = 10
    activation_energy = 17.8E3
    one_over_T0 = 0.003354
  []
  [rate_Zoisite]
    type = GeochemistryKineticRate
    kinetic_species_name = Zoisite
    intrinsic_rate_constant = 1E-16
    multiply_by_mass = true
    area_quantity = 10
    activation_energy = 66.1E3
    one_over_T0 = 0.003354
  []
  [definition]
    type = GeochemicalModelDefinition
    database_file = '../../../../geochemistry/database/moose_geochemdb.json'
    basis_species = 'H2O H+ Na+ K+ Ca++ Mg++ SiO2(aq) Al+++ Cl- SO4-- HCO3-'
    remove_all_extrapolated_secondary_species = true
    kinetic_minerals = 'Albite Anhydrite Anorthite Calcite Chalcedony Clinochl-7A Illite K-feldspar Kaolinite Quartz Paragonite Phlogopite Zoisite Laumontite'
    kinetic_rate_descriptions = 'rate_Albite rate_Anhydrite rate_Anorthite rate_Calcite rate_Chalcedony rate_Clinochl-7A rate_Illite rate_K-feldspar rate_Kaolinite rate_Quartz rate_Paragonite rate_Phlogopite rate_Zoisite rate_Laumontite'
  []
[]

[TimeDependentReactionSolver]
  model_definition = definition
  geochemistry_reactor_name = reactor
  charge_balance_species = 'Cl-'
  constraint_species = 'H2O              H+                  Na+                K+                 Ca++               Mg++                SiO2(aq)           Al+++               Cl-              SO4--               HCO3-'
# Following numbers are from water_60_to_220degC_out.csv
  constraint_value = '  1.0006383866109  9.5165072498215e-07 0.100020379171     0.0059389061065    0.011570884507621  4.6626763057447e-06 0.0045110404925255 5.8096968688789e-17 0.13500708594394 6.6523540147676e-05 7.7361407898089e-05'
  constraint_meaning = 'kg_solvent_water free_concentration  free_concentration free_concentration free_concentration free_concentration  free_concentration free_concentration  bulk_composition free_concentration  free_concentration'
  constraint_unit = '   kg               molal               molal              molal              molal              molal               molal              molal               moles            molal               molal'
  initial_temperature = 220
  temperature = 220
  kinetic_species_name = '         Albite             Anorthite          K-feldspar         Quartz             Phlogopite         Paragonite         Calcite            Anhydrite          Chalcedony         Illite             Kaolinite          Clinochl-7A        Zoisite            Laumontite'
  kinetic_species_initial_value = '4.324073236492E+02 4.631370307325E+01 2.685015418378E+02 7.720095013956E+02 1.235192062541E+01 7.545461404965E-01 4.234651808835E-04 4.000485907930E-04 4.407616361072E+00 1.342524904876E+01 1.004823151125E+00 4.728132387707E-01 7.326007326007E-01 4.818116116598E-01'
  kinetic_species_unit = '         moles              moles              moles              moles              moles              moles              moles              moles              moles              moles              moles              moles              moles              moles'
  evaluate_kinetic_rates_always = true # otherwise will easily "run out" of dissolving species
  ramp_max_ionic_strength_initial = 0 # max_ionic_strength in such a simple problem does not need ramping
  execute_console_output_on = ''
[]

[Executioner]
  type = Transient
  [TimeStepper]
    type = FunctionDT
    function = 'max(1E2, 0.1 * t)'
  []
  end_time = 4E11
[]

[GlobalParams]
  point = '0 0 0'
[]

[Postprocessors]
  [cm3_Albite]
    type = PointValue
    variable = 'free_cm3_Albite'
  []
  [cm3_Anhydrite]
    type = PointValue
    variable = 'free_cm3_Anhydrite'
  []
  [cm3_Anorthite]
    type = PointValue
    variable = 'free_cm3_Anorthite'
  []
  [cm3_Calcite]
    type = PointValue
    variable = 'free_cm3_Calcite'
  []
  [cm3_Chalcedony]
    type = PointValue
    variable = 'free_cm3_Chalcedony'
  []
  [cm3_Clinochl-7A]
    type = PointValue
    variable = 'free_cm3_Clinochl-7A'
  []
  [cm3_Illite]
    type = PointValue
    variable = 'free_cm3_Illite'
  []
  [cm3_K-feldspar]
    type = PointValue
    variable = 'free_cm3_K-feldspar'
  []
  [cm3_Kaolinite]
    type = PointValue
    variable = 'free_cm3_Kaolinite'
  []
  [cm3_Quartz]
    type = PointValue
    variable = 'free_cm3_Quartz'
  []
  [cm3_Paragonite]
    type = PointValue
    variable = 'free_cm3_Paragonite'
  []
  [cm3_Phlogopite]
    type = PointValue
    variable = 'free_cm3_Phlogopite'
  []
  [cm3_Zoisite]
    type = PointValue
    variable = 'free_cm3_Zoisite'
  []
  [cm3_Laumontite]
    type = PointValue
    variable = 'free_cm3_Laumontite'
  []
  [cm3_mineral]
    type = LinearCombinationPostprocessor
    pp_names = 'cm3_Albite cm3_Anhydrite cm3_Anorthite cm3_Calcite cm3_Chalcedony cm3_Clinochl-7A cm3_Illite cm3_K-feldspar cm3_Kaolinite cm3_Quartz cm3_Paragonite cm3_Phlogopite cm3_Zoisite cm3_Laumontite'
    pp_coefs = '1 1 1 1 1 1 1 1 1 1 1 1 1 1'
  []
  [pH]
    type = PointValue
    variable = 'pH'
  []
  [kg_solvent_H2O]
    type = PointValue
    variable = 'kg_solvent_H2O'
  []
[]

[Outputs]
  csv = true
[]


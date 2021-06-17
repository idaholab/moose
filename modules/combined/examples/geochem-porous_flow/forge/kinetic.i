# Simulation to check that the output of water_60_to_220degC is indeed at equilibrium with the mineral assemblage.
# The initial mole numbers of the kinetic species are unimportant for this simulation, but are chosen to be consistent with other input files.  The numerical values are such that:
# - the mass fractions are: Albite 0.44; Anorthite 0.05; K-feldspar 0.29; Quartz 0.18, Phlgoptite 0.04 with trace amounts of Calcite and Anhydrite.  These are similar to that measured in bulk X-ray diffraction results of 10 samples from well 58-32, assuming that "plagioclase feldspar" consists of Albite and Anorthite in the ratio 9:1, and that Biotite is Phlogoptite, and the 2% Illite is added to Phlogoptite.  Precisely:
# - it is assumed that water_60_to_220degC consists of 1 litre of water (there is 1kg of solvent water) and that the porosity is 0.01, so the amount of rock should be 99000cm^3
# - the cm^3 of the trace minerals Calcite and Anhydrite is exactly given by water_60_to_220degC (0.016 and 0.018 respectively)
# - the total mineral volume is 99000cm^3, so that the porosity is 0.01
# - see initial_kinetic_moles.xlsx for the remaining mole numbers
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
    kinetic_minerals = 'Albite Anhydrite Anorthite Calcite Chalcedony Clinochl-7A Illite K-feldspar Kaolinite Quartz Paragonite Phlogopite'
    kinetic_rate_descriptions = 'rate_Albite rate_Anhydrite rate_Anorthite rate_Calcite rate_Chalcedony rate_Clinochl-7A rate_Illite rate_K-feldspar rate_Kaolinite rate_Quartz rate_Paragonite rate_Phlogopite'
  []
[]

[TimeDependentReactionSolver]
  model_definition = definition
  geochemistry_reactor_name = reactor
  charge_balance_species = 'Cl-'
  constraint_species = 'H2O              H+                  Na+              K+                 Ca++              Mg++                SiO2(aq)           Al+++               Cl-                SO4--               HCO3-'
# Following numbers are from water_60_to_220degC_out.csv
  constraint_value = '  1.0006383866109  9.5165072498215e-07 0.100020379171   0.0059389061065    0.011570884507621 4.6626763057447e-06 0.0045110404925255 5.8096968688789e-17 0.13500708594394   6.6523540147676e-05 7.7361407898089e-05'
  constraint_meaning = 'kg_solvent_water free_concentration       free_concentration    free_concentration     free_concentration   free_concentration       free_concentration      free_concentration       bulk_composition free_concentration       free_concentration'
  constraint_unit = '   kg               molal               molal            molal              molal           molal              molal              molal               moles              molal               molal'
  initial_temperature = 220
  temperature = 220
  kinetic_species_name = '         Albite           Anorthite       K-feldspar      Quartz           Phlogopite         Paragonite Calcite     Anhydrite   Chalcedony Illite Kaolinite Clinochl-7A'
  kinetic_species_initial_value = '4.3511787009E+02 4.660402064E+01 2.701846444E+02 7.7684884497E+02 2.4858697344E+01   1E-10      0.000423465 0.000400049 1E-10 1E-10 1E-10 1E-10'
  kinetic_species_unit = '         moles            moles           moles           moles            moles              moles      moles       moles       moles      moles  moles     moles'
  evaluate_kinetic_rates_always = true # otherwise will easily "run out" of dissolving species
  ramp_max_ionic_strength_initial = 0 # max_ionic_strength in such a simple problem does not need ramping
  mol_cutoff = 0.1
  execute_console_output_on = ''
[]

[Executioner]
  type = Transient
  [TimeStepper]
    type = FunctionDT
    function = '1E8'
  []
  end_time = 4E8
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
  [cm3_mineral]
    type = LinearCombinationPostprocessor
    pp_names = 'cm3_Albite cm3_Anhydrite cm3_Anorthite cm3_Calcite cm3_Chalcedony cm3_Clinochl-7A cm3_Illite cm3_K-feldspar cm3_Kaolinite cm3_Quartz cm3_Paragonite cm3_Phlogopite'
    pp_coefs = '1 1 1 1 1 1 1 1 1 1 1 1'
  []
[]

[Outputs]
  csv = true
[]


# Simulation to assess possible changes in the reservoir.  The rock composition from natural_reservoir.i is mixed with the water from water_3.i  Note that the free_concentration values are used from water_3.i and that composition is held fixed throughout this entire simulation.  This models water_3 continually flushing through the rock mineral assemblage: as soon as a mineral dissolves the aqueous components are swept away and replaced by a new batch of water_3; as soon as mineral precipitates more water_3 sweeps into the system providing a limitless source of aqueous components (in set ratios) at 70degC
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
  constraint_species = 'H2O              H+                  Na+                K+                  Ca++                Mg++               SiO2(aq)            Al+++               Cl-                SO4--               HCO3-'
# Following numbers are from water_3_out.csv
  constraint_value = '  0.99999999549877 8.0204734722945e-07 0.0001319920398478 2.8097346859027e-05 7.7328020546464e-05 2.874602030221e-05 0.00027284654762868 4.4715524787497e-12 0.0002253530818877 1.0385772502298e-05 0.00012427759434288'
  constraint_meaning = 'kg_solvent_water free_concentration       free_concentration    free_concentration      free_concentration     free_concentration       free_concentration      free_concentration       bulk_composition free_concentration       free_concentration'
  constraint_unit = '   kg               molal               molal            molal              molal             molal               molal              molal               moles              molal               molal'
  initial_temperature = 70
  temperature = 70
  close_system_at_time = 1E20 # keep the free molalities specified above for all time
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
    function = 'max(1E6, 0.3 * t)'
  []
  end_time = 4E11
[]

[GlobalParams]
  point = '0 0 0'
[]

[Postprocessors]
  [temperature]
    type = PointValue
    variable = 'solution_temperature'
  []
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
[]

[Outputs]
  csv = true
[]


# Alkali flushing of a reservoir (an example of flushing): adding NaOH solution
[UserObjects]
  [rate_quartz]
    type = GeochemistryKineticRate
    kinetic_species_name = Quartz
    intrinsic_rate_constant = 1.5552E-13 # 1.8E-18mol/s/cm^2 = 1.5552E-13mol/day/cm^2
    multiply_by_mass = true
    area_quantity = 1000
    promoting_species_names = "H+"
    promoting_indices = "-0.5"
  []
  [definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O H+ Cl- Na+ Ca++ HCO3- Mg++ K+ Al+++ SiO2(aq)"
    equilibrium_minerals = "Analcime Calcite Dawsonite Dolomite-ord Gibbsite Kaolinite Muscovite Paragonite Phlogopite"
    kinetic_minerals = "Quartz"
    kinetic_rate_descriptions = "rate_quartz"
  []
[]

[TimeDependentReactionSolver]
  model_definition = definition
  geochemistry_reactor_name = reactor
  charge_balance_species = "Cl-"
  swap_into_basis = "Calcite Dolomite-ord Muscovite Kaolinite"
  swap_out_of_basis = "HCO3- Mg++ K+ Al+++"
  constraint_species = "H2O              H+            Cl-              Na+              Ca++             Calcite      Dolomite-ord Muscovite    Kaolinite    SiO2(aq)"
  constraint_value = "  1.0              -5            1.0              1.0              0.2              9.88249      3.652471265  1.2792268    1.2057878    0.000301950628974"
  constraint_meaning = "kg_solvent_water log10activity bulk_composition bulk_composition bulk_composition free_mineral free_mineral free_mineral free_mineral free_concentration"
  constraint_unit = "   kg               dimensionless moles            moles            moles            moles        moles        moles        moles        molal"
  initial_temperature = 70.0
  temperature = 70.0
  kinetic_species_name = Quartz
  kinetic_species_initial_value = 226.992243
  kinetic_species_unit = moles
  evaluate_kinetic_rates_always = true # implicit time-marching used for stability
  close_system_at_time = 0.0
  remove_fixed_activity_name = "H+"
  remove_fixed_activity_time = 0.0
  mode = 3 # flush through the NaOH solution specified below:
  source_species_names = "H2O   Na+  OH-"
  source_species_rates = "27.75 0.25 0.25" # 1kg water/2days = 27.75moles/day.  0.5mol Na+/2days = 0.25mol/day
  ramp_max_ionic_strength_initial = 0 # max_ionic_strength in such a simple problem does not need ramping
  stoichiometric_ionic_str_using_Cl_only = true
  execute_console_output_on = '' # only CSV output for this test
  abs_tol = 1E-12
[]

[Executioner]
  type = Transient
  dt = 1
  end_time = 20 # measured in days
[]

[GlobalParams]
  point = '0 0 0'
[]
[Postprocessors]
  [pH]
    type = PointValue
    variable = "pH"
  []
  [cm3_Analcime]
    type = PointValue
    variable = free_cm3_Analcime
  []
  [cm3_Calcite]
    type = PointValue
    variable = free_cm3_Calcite
  []
  [cm3_Dawsonite]
    type = PointValue
    variable = free_cm3_Dawsonite
  []
  [cm3_Dolomite]
    type = PointValue
    variable = free_cm3_Dolomite-ord
  []
  [cm3_Gibbsite]
    type = PointValue
    variable = free_cm3_Gibbsite
  []
  [cm3_Kaolinite]
    type = PointValue
    variable = free_cm3_Kaolinite
  []
  [cm3_Muscovite]
    type = PointValue
    variable = free_cm3_Muscovite
  []
  [cm3_Paragonite]
    type = PointValue
    variable = free_cm3_Paragonite
  []
  [cm3_Phlogopite]
    type = PointValue
    variable = free_cm3_Phlogopite
  []
  [cm3_Quartz]
    type = PointValue
    variable = free_cm3_Quartz
  []
[]

[Outputs]
  csv = true
[]

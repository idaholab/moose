# Alkali flushing of a reservoir (an example of flushing): adding NaOH
# To determine the initial constraint_values, run flushing_equilibrium_at70degC.i
# Note that flushing_equilibrium_at70degC.i will have to be re-run when temperature-dependence has been added to geochemistry
# Note that Dawsonite is currently not included as an equilibrium_mineral, otherwise it is supersaturated in the initial configuration, so precipitates.  Bethke does not report this in Fig30.4, so I assume it is due to temperature dependence
[GlobalParams]
  point = '0 0 0'
[]

[TimeDependentReactionSolver]
  model_definition = definition
  geochemistry_reactor_name = reactor
  charge_balance_species = "Cl-"
  swap_into_basis = "Calcite Dolomite-ord Muscovite Kaolinite"
  swap_out_of_basis = "HCO3- Mg++ K+ Al+++"
  constraint_species = "H2O H+   Cl-       Na+       Ca++       Calcite   Dolomite-ord Muscovite Kaolinite SiO2(aq)"
  constraint_value = "  1.0 1E-5 2.1716946 1.0288941 0.21650572 10.177537 3.6826177    1.320907  1.1432682 6.318e-05"
  constraint_meaning = "kg_solvent_water activity bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition free_concentration"
  constraint_unit = "   kg              dimensionless moles          moles              moles              moles              moles              moles              moles              molal"
  initial_temperature = 70.0
  temperature = 70.0
  kinetic_species_name = Quartz
  kinetic_species_initial_value = 226.992243
  kinetic_species_unit = moles
  evaluate_kinetic_rates_always = true # implicit time-marching used for stability
  ramp_max_ionic_strength_initial = 0 # max_ionic_strength in such a simple problem does not need ramping
  close_system_at_time = 0.0
  remove_fixed_activity_name = "H+"
  remove_fixed_activity_time = 0.0
  mode = 3 # flush through the NaOH solution specified below:
  source_species_names = "H2O    Na+  OH-"
  source_species_rates = "27.755 0.25 0.25" # 1kg water/2days = 27.755moles/day.  0.5mol Na+/2days = 0.25mol/day
[]

[UserObjects]
  [rate_quartz]
    type = GeochemistryKineticRate
    kinetic_species_name = Quartz
    intrinsic_rate_constant = 1.3824E-13 # 1.6E-19mol/s/cm^2 = 1.3824E-13mol/day/cm^2
    multiply_by_mass = true
    area_quantity = 1000
    promoting_species_names = "H+"
    promoting_indices = "-0.5"
  []
  [definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O H+ Cl- Na+ Ca++ HCO3- Mg++ K+ Al+++ SiO2(aq)"
    equilibrium_minerals = "Calcite Dolomite-ord Muscovite Kaolinite Paragonite Analcime Phlogopite Tridymite" # Dawsonite
    kinetic_minerals = "Quartz"
    kinetic_rate_descriptions = "rate_quartz"
  []
[]

[AuxVariables]
  [diss_rate]
  []
[]
[AuxKernels]
  [diss_rate]
    type = ParsedAux
    coupled_variables = mol_change_Quartz
    expression = '-mol_change_Quartz / 1.0' # 1.0 = timestep size
    variable = diss_rate
  []
[]

[Postprocessors]
  [pH]
    type = PointValue
    variable = "pH"
  []
  [rate_mole_per_day]
    type = PointValue
    variable = diss_rate
  []
  [cm3_Calcite]
    type = PointValue
    variable = free_cm3_Calcite
  []
  [cm3_Dolomite]
    type = PointValue
    variable = free_cm3_Dolomite-ord
  []
  [cm3_Muscovite]
    type = PointValue
    variable = free_cm3_Muscovite
  []
  [cm3_Kaolinite]
    type = PointValue
    variable = free_cm3_Kaolinite
  []
  [cm3_Quartz]
    type = PointValue
    variable = free_cm3_Quartz
  []
  [cm3_Paragonite]
    type = PointValue
    variable = free_cm3_Paragonite
  []
  [cm3_Analcime]
    type = PointValue
    variable = free_cm3_Analcime
  []
  [cm3_Phlogopite]
    type = PointValue
    variable = free_cm3_Phlogopite
  []
  [cm3_Tridymite]
    type = PointValue
    variable = free_cm3_Tridymite
  []
[]

[Executioner]
  type = Transient
  dt = 1
  end_time = 20 # measured in days
[]

[Outputs]
  csv = true
[]

# Example of quartz deposition in a fracture, as the temperature is reduced from 300degC to 25degC
# The initial free molality of SiO2(aq) is determined using quartz_equilibrium_at300degC
[GlobalParams]
  point = '0 0 0'
[]

[TimeDependentReactionSolver]
  model_definition = definition
  geochemistry_reactor_name = reactor
  charge_balance_species = "Cl-"
  constraint_species = "H2O              Na+              Cl-              SiO2(aq)"
  constraint_value = "  1.0              1E-10            1E-10            0.009722905"
  constraint_meaning = "kg_solvent_water bulk_composition bulk_composition free_concentration"
  constraint_unit = "   kg               moles            moles            molal"
  initial_temperature = 300.0
  temperature = temp_controller
  kinetic_species_name = Quartz
  kinetic_species_initial_value = 400
  kinetic_species_unit = g
  ramp_max_ionic_strength_initial = 0 # max_ionic_strength in such a simple problem does not need ramping
  add_aux_pH = false # there is no H+ in this system
  evaluate_kinetic_rates_always = true # implicit time-marching used for stability
  execute_console_output_on = '' # only CSV output used in this example
[]

[UserObjects]
  [rate_quartz]
    type = GeochemistryKineticRate
    kinetic_species_name = Quartz
    intrinsic_rate_constant = 7.4112E2 # 2.35E-5mol/s/cm^2 = 7.411E2mol/yr/cm^2
    multiply_by_mass = true
    area_quantity = 1
    activation_energy = 72800.0
  []
  [definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O SiO2(aq) Na+ Cl-"
    kinetic_minerals = "Quartz"
    kinetic_rate_descriptions = "rate_quartz"
  []
[]


[Executioner]
  type = Transient
  dt = 0.02
  end_time = 1 # measured in years
[]

[AuxVariables]
  [temp_controller]
  []
  [diss_rate]
  []
[]
[AuxKernels]
  [temp_controller_auxk]
    type = FunctionAux
    function = '300 - 275 * t'
    variable = temp_controller
    execute_on = 'timestep_begin'
  []
  [diss_rate]
    type = ParsedAux
    coupled_variables = mol_change_Quartz
    expression = '-mol_change_Quartz / 0.02' # 0.02 = timestep size
    variable = diss_rate
  []
[]
[Postprocessors]
  [mg_per_kg_sio2]
    type = PointValue
    variable = "mg_per_kg_SiO2(aq)"
  []
  [rate_mole_per_year]
    type = PointValue
    variable = diss_rate
  []
  [temperature]
    type = PointValue
    variable = "solution_temperature"
  []
[]
[Outputs]
  csv = true
[]

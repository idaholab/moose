#Temperature is changed and precipitates are observed
[TimeDependentReactionSolver]
  model_definition = definition
  geochemistry_reactor_name = reactor
  swap_out_of_basis = "Al+++ K+ H+ SiO2(aq)"
  swap_into_basis = "Albite Maximum Muscovite Quartz"
  charge_balance_species = "Cl-"
  constraint_species = "H2O Muscovite Na+ Cl- Albite  Maximum Quartz"
  constraint_value = "  1.0 0.03553   1.0 1.0 0.19986 0.09196 0.08815"
  constraint_meaning = "kg_solvent_water free_moles_mineral_species moles_bulk_species moles_bulk_species free_moles_mineral_species free_moles_mineral_species free_moles_mineral_species"
  ramp_max_ionic_strength = 0
  close_system_at_time = 0
  initial_temperature = 300
  temperature = temperature
  execute_console_output_on = 'final'
[]

[AuxVariables]
  [./temperature]
  [../]
[]
[AuxKernels]
  [./temperature]
    type = FunctionAux
    variable = temperature
    function = '300 - t'
    execute_on = 'timestep_begin' # so that it is correct when we solve the system
  [../]
[]

[Postprocessors]
  [./cm3_Max_Micro]
    type = PointValue
    point = '0 0 0'
    variable = 'free_cm3_Maximum'
  [../]
  [./cm3_Albite]
    type = PointValue
    point = '0 0 0'
    variable = 'free_cm3_Albite'
  [../]
  [./cm3_Muscovite]
    type = PointValue
    point = '0 0 0'
    variable = 'free_cm3_Muscovite'
  [../]
  [./cm3_Quartz]
    type = PointValue
    point = '0 0 0'
    variable = 'free_cm3_Quartz'
  [../]
[]

[Executioner]
  type = Transient
  dt = 100
  end_time = 275
[]

[UserObjects]
  [./definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O H+ Na+ Cl- Al+++ K+ SiO2(aq)"
    equilibrium_minerals = "Albite Maximum Muscovite Quartz"
  [../]
[]


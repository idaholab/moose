# Example of quartz dissolution.
[GlobalParams]
  point = '0 0 0'
[]

[TimeDependentReactionSolver]
  model_definition = definition
  geochemistry_reactor_name = reactor
  charge_balance_species = "Cl-"
  constraint_species = "H2O H+ Cl- SiO2(aq)"
  constraint_value = "  1.0 1E-5 1E-5 1E-9"
  constraint_meaning = "kg_solvent_water moles_bulk_species moles_bulk_species free_molality"
  initial_temperature = 100.0
  temperature = 100.0
  kinetic_species_name = Quartz
  kinetic_species_initial_moles = 83.216414271 # Quartz has 60.0843g/mol
  ramp_max_ionic_strength_initial = 0 # max_ionic_strength in such a simple problem does not need ramping
  close_system_at_time = 0
[]

[UserObjects]
  [./rate_quartz]
    type = GeochemistryKineticRate
    kinetic_species_name = Quartz
    intrinsic_rate_constant = 1.728E-10 # 1.0E-15mol/s/cm^2 = 1.728E-10mol/day/cm^2
    multiply_by_mass = true
    area_quantity = 1000
  [../]
  [./definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O SiO2(aq) H+ Cl-"
    kinetic_minerals = "Quartz"
    kinetic_rate_descriptions = "rate_quartz"
  [../]
[]

[AuxVariables]
  [./diss]
  [../]
  [./diss_rate]
  [../]
[]
[AuxKernels]
  [./diss]
    type = ParsedAux
    args = moles_Quartz
    function = '83.216414271 - moles_Quartz'
    variable = diss
  [../]
  [./diss_rate]
    type = ParsedAux
    args = mol_change_Quartz
    function = '-mol_change_Quartz / 0.1' # 0.1 = timestep size
    variable = diss_rate
  [../]
[]

[Postprocessors]
  [./dissolved_moles]
    type = PointValue
    variable = diss
  [../]
  [./rate_mole_per_day]
    type = PointValue
    variable = diss_rate
  [../]
[]

[Executioner]
  type = Transient
  dt = 0.1
  end_time = 5.0
[]

[Outputs]
  csv = true
[]

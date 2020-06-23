# Example of kinetically-controlled dissolution of albite into an acidic solution
[GlobalParams]
  point = '0 0 0'
[]

[TimeDependentReactionSolver]
  model_definition = definition
  geochemistry_reactor_name = reactor
  charge_balance_species = "Cl-"
  constraint_species = "H2O H+        Cl- Na+ SiO2(aq) Al+++"
  constraint_value = "  1.0 0.0316228 0.1 0.1 1E-6     1E-6"
  constraint_meaning = "kg_solvent_water activity moles_bulk_species free_molality free_molality free_molality"
  initial_temperature = 70.0
  temperature = 70.0
  kinetic_species_name = Albite
  kinetic_species_initial_moles = 0.953387 # Albite has 262.2230g/mol
  ramp_max_ionic_strength_initial = 0 # max_ionic_strength in such a simple problem does not need ramping
  close_system_at_time = 0
  evaluate_kinetic_rates_always = true # implicit time-marching used for stability
[]

[UserObjects]
  [./rate_albite]
    type = GeochemistryKineticRate
    kinetic_species_name = Albite
    intrinsic_rate_constant = 5.4432E-8 # 6.3E-13mol/s/cm^2 = 5.4432E-8mol/day/cm^2
    multiply_by_mass = true
    area_quantity = 1000
    promoting_species_names = "H+"
    promoting_species_indices = "1.0"
  [../]
  [./definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O H+ Cl- Na+ SiO2(aq) Al+++"
    kinetic_minerals = "Albite"
    kinetic_rate_descriptions = "rate_albite"
  [../]
[]

[AuxVariables]
  [./mole_change_albite]
  [../]
  [./diss_rate]
  [../]
[]
[AuxKernels]
  [./mole_change_albite]
    type = ParsedAux
    args = moles_Albite
    function = 'moles_Albite - 0.953387'
    variable = mole_change_albite
  [../]
  [./diss_rate]
    type = ParsedAux
    args = mol_change_Albite
    function = '-mol_change_Albite / 1.0' # 1.0 = timestep size
    variable = diss_rate
  [../]
[]

[Postprocessors]
  [./mole_change_Albite]
    type = PointValue
    variable = "mole_change_albite"
  [../]
  [./rate_mole_per_day]
    type = PointValue
    variable = diss_rate
  [../]
[]

[Executioner]
  type = Transient
  dt = 1
  end_time = 30 # measured in days
[]

[Outputs]
  csv = true
[]

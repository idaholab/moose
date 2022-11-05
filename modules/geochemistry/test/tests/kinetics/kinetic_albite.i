# Example of kinetically-controlled dissolution of albite into an acidic solution
[TimeDependentReactionSolver]
  model_definition = definition
  geochemistry_reactor_name = reactor
  charge_balance_species = "Cl-"
  constraint_species = "H2O              H+            Cl-              Na+              SiO2(aq)           Al+++"
  constraint_value = "  1.0              -1.5          0.1              0.1              1E-6               1E-6"
  constraint_meaning = "kg_solvent_water log10activity bulk_composition bulk_composition free_concentration free_concentration"
  constraint_unit = "   kg               dimensionless moles            moles            molal              molal"
  initial_temperature = 70.0
  temperature = 70.0
  kinetic_species_name = Albite
  kinetic_species_initial_value = 250
  kinetic_species_unit = g
  evaluate_kinetic_rates_always = true # implicit time-marching used for stability
  ramp_max_ionic_strength_initial = 0 # max_ionic_strength in such a simple problem does not need ramping
  stoichiometric_ionic_str_using_Cl_only = true # for comparison with GWB
  execute_console_output_on = '' # only CSV output for this example
[]

[UserObjects]
  [rate_albite]
    type = GeochemistryKineticRate
    kinetic_species_name = Albite
    intrinsic_rate_constant = 5.4432E-8 # 6.3E-13mol/s/cm^2 = 5.4432E-8mol/day/cm^2
    multiply_by_mass = true
    area_quantity = 1000
    promoting_species_names = "H+"
    promoting_indices = "1.0"
  []
  [definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O H+ Cl- Na+ SiO2(aq) Al+++"
    kinetic_minerals = "Albite"
    kinetic_rate_descriptions = "rate_albite"
  []
[]

[Executioner]
  type = Transient
  dt = 5
  end_time = 30 # measured in days
[]

[AuxVariables]
  [mole_change_albite]
  []
[]
[AuxKernels]
  [mole_change_albite]
    type = ParsedAux
    coupled_variables = moles_Albite
    expression = 'moles_Albite - 0.953387'
    variable = mole_change_albite
  []
[]
[Postprocessors]
  [mole_change_Albite]
    type = PointValue
    point = '0 0 0'
    variable = "mole_change_albite"
  []
[]
[Outputs]
  csv = true
[]

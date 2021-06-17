[UserObjects]
  [definition]
    type = GeochemicalModelDefinition
    database_file = "small_database.json"
    basis_species = "H2O SiO2(aq) Na+ Cl-"
    equilibrium_minerals = "QuartzUnlike"
  []
[]

[TimeIndependentReactionSolver]
  model_definition = definition
  geochemistry_reactor_name = reactor
  charge_balance_species = "Cl-"
  swap_out_of_basis = "SiO2(aq)"
  swap_into_basis = QuartzUnlike
  constraint_species = "H2O              Na+              Cl-              QuartzUnlike"
  constraint_value = "  1.0              0.1              0.1              396.685"
  constraint_meaning = "kg_solvent_water bulk_composition bulk_composition free_mineral"
  constraint_unit = "   kg               moles            moles            moles"
  temperature = 50.0
  ramp_max_ionic_strength_initial = 0 # max_ionic_strength in such a simple problem does not need ramping
  add_aux_pH = false # there is no H+ in this system
  precision = 12
[]

[Postprocessors]
  [free_moles_SiO2]
    type = PointValue
    point = '0 0 0'
    variable = 'molal_SiO2(aq)'
  []
[]

[Outputs]
  csv = true
[]

#Exception test: species does not exist
[TimeIndependentReactionSolver]
  model_definition = definition
  charge_balance_species = "Cl-"
  constraint_species = "H2O H+ Cl-"
  constraint_value = "  1.0 1.0 1.0"
  constraint_meaning = "kg_solvent_water bulk_composition bulk_composition"
  constraint_unit = "kg moles moles"
[]

[UserObjects]
  [definition]
    type = GeochemicalModelDefinition
    database_file = "../../database/ferric_hydroxide_sorption.json"
    basis_species = "H2O H+ Cl-"
  []
[]

[AuxVariables]
  [error]
  []
[]

[AuxKernels]
  [error]
    type = GeochemistryQuantityAux
    species = Na
    reactor = geochemistry_reactor
    variable = error
  []
[]




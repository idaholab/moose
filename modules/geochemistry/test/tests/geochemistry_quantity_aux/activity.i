#Extract activity
[TimeIndependentReactionSolver]
  model_definition = definition
  charge_balance_species = "Cl-"
  constraint_species = "H2O H+ Cl-"
  constraint_value = "  1.0 1.0E-2 1.0E-2"
  constraint_meaning = "kg_solvent_water activity bulk_composition"
  constraint_unit = "kg dimensionless moles"
  ramp_max_ionic_strength_initial = 0
[]

[UserObjects]
  [definition]
    type = GeochemicalModelDefinition
    database_file = "../../database/ferric_hydroxide_sorption.json"
    basis_species = "H2O H+ Cl-"
  []
[]

[AuxVariables]
  [the_aux]
  []
[]

[AuxKernels]
  [the_aux]
    type = GeochemistryQuantityAux
    species = 'H+'
    reactor = geochemistry_reactor
    variable = the_aux
    quantity = activity
  []
[]

[Postprocessors]
  [value]
    type = PointValue
    point = '0 0 0'
    variable = the_aux
  []
  [value_from_action]
    type = PointValue
    point = '0 0 0'
    variable = "activity_H+"
  []
[]

[Outputs]
  csv = true
[]

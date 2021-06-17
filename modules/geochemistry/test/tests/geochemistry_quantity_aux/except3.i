#Exception test: inappropriate quantity
[TimeIndependentReactionSolver]
  model_definition = definition
  charge_balance_species = "Cl-"
  constraint_species = "H2O H+ Cl- Fe+++ >(s)FeOH >(w)FeOH"
  constraint_value = "  1.0 1.0 1.0 1.0 1.0 1.0"
  constraint_meaning = "kg_solvent_water bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition"
  constraint_unit = "kg moles moles moles moles moles"
[]

[UserObjects]
  [definition]
    type = GeochemicalModelDefinition
    database_file = "../../database/ferric_hydroxide_sorption.json"
    basis_species = "H2O H+ Cl- Fe+++ >(s)FeOH >(w)FeOH"
    equilibrium_minerals = "Fe(OH)3(ppd)"
  []
[]

[AuxVariables]
  [error]
  []
[]

[AuxKernels]
  [error]
    type = GeochemistryQuantityAux
    species = "Fe(OH)3(ppd)"
    reactor = geochemistry_reactor
    variable = error
    quantity = molal
  []
[]




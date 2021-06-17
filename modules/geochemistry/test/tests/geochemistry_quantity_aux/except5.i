#Exception test: inappropriate quantity
[TimeDependentReactionSolver]
  model_definition = definition
  charge_balance_species = "Cl-"
  constraint_species = "H2O H+ Cl- Fe+++ >(s)FeOH >(w)FeOH"
  constraint_value = "  1.0 1.0 1.0 1.0 1.0 1.0"
  constraint_meaning = "kg_solvent_water bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition"
  constraint_unit = "kg moles moles moles moles moles"
  kinetic_species_name = "Fe(OH)3(ppd)"
  kinetic_species_initial_value = "1.0"
  kinetic_species_unit = "moles"
[]

[UserObjects]
  [definition]
    type = GeochemicalModelDefinition
    database_file = "../../database/ferric_hydroxide_sorption.json"
    basis_species = "H2O H+ Cl- Fe+++ >(s)FeOH >(w)FeOH"
    kinetic_minerals = "Fe(OH)3(ppd)"
  []
[]

[Executioner]
  type = Steady
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
    quantity = kinetic_moles
  []
[]




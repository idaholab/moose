#Exception test: inappropriate quantity
[TimeIndependentReactionSolver]
  model_definition = definition
  charge_balance_species = "Cl-"
  constraint_species = "H2O H+ Cl- Fe+++"
  constraint_value = "  1.0 1.0 1.0 1.0"
  constraint_meaning = "kg_solvent_water moles_bulk_species moles_bulk_species moles_bulk_species"
[]

[UserObjects]
  [./definition]
    type = GeochemicalModelDefinition
    database_file = "../../database/ferric_hydroxide_sorption.json"
    basis_species = "H2O H+ Cl- Fe+++"
    equilibrium_minerals = "Fe(OH)3(ppd)_nosorb"
  [../]
[]

[AuxVariables]
  [./error]
  [../]
[]

[AuxKernels]
  [./error]
    type = GeochemistryQuantityAux
    species = "Fe(OH)3(ppd)_nosorb"
    reactor = geochemistry_reactor
    variable = error
    quantity = surface_charge
  [../]
[]

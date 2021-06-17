# Find temperature at equilibrium for reactions involving gypsum
[GeochemicalModelInterrogator]
  model_definition = definition
  swap_out_of_basis = "Ca++"
  swap_into_basis = "  Anhydrite"
  activity_species = "H2O"
  activity_values = "1.0"
  temperature = 25
  equilibrium_species = Gypsum
  interrogation = eqm_temperature
[]

[UserObjects]
  [definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O Ca++ SO4--"
    equilibrium_minerals = "Gypsum Anhydrite"
  []
[]


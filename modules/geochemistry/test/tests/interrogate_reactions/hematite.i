# Compute pH and pe values for an equilibrium reaction involving the mineral hematite
[GeochemicalModelInterrogator]
  model_definition = definition
  swap_out_of_basis = "O2(aq)"
  swap_into_basis = "  e-"
  equilibrium_species = Hematite
  activity_species = 'H2O Fe++'
  activity_values = '1 1E-10'
  interrogation = activity
[]

[UserObjects]
  [./definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O Fe++ H+ O2(aq)"
    equilibrium_minerals = "Hematite"
  [../]
[]


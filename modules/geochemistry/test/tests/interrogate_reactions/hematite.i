# Compute pH values for an equilibrium reaction involving the mineral hematite
[GeochemicalModelInterrogator]
  model_definition = definition
  equilibrium_species = Hematite
  activity_species = 'H2O Fe++'
  activity_values = '1 1E-10'
  interrogation = activity
[]

[UserObjects]
  [definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O Fe++ H+ O2(aq)"
    equilibrium_minerals = "Hematite"
    piecewise_linear_interpolation = true # to get exact logK at 25degC with no best-fit interpolation
  []
[]


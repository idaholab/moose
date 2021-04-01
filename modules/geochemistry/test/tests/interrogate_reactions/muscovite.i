# Output activity ratios for reactions involving muscovite
[GeochemicalModelInterrogator]
  model_definition = definition
  swap_out_of_basis = "Al+++"
  swap_into_basis = "  Kaolinite"
  activity_species = "H2O"
  activity_values = "1.0"
  equilibrium_species = Muscovite
  interrogation = activity
[]

[UserObjects]
  [definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O K+ Al+++ SiO2(aq) H+"
    equilibrium_minerals = "Muscovite Kaolinite"
    piecewise_linear_interpolation = true # to get exact logK at 25degC with no best-fit interpolation
  []
[]


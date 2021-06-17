[GeochemicalModelInterrogator]
  model_definition = definition
  swap_out_of_basis = "Al+++"
  swap_into_basis = "  Quartz"
[]

[UserObjects]
  [definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O K+ Al+++ SiO2(aq) H+"
    equilibrium_minerals = "Muscovite Quartz Maximum Tridymite Amrph^silica"
  []
[]


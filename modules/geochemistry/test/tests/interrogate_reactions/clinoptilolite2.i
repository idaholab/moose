# Outputs activity ratios for reactions involving Clinoptil-Ca
[GeochemicalModelInterrogator]
  model_root = root
  swap_out_of_basis = "Al+++    SiO2(aq)"
  swap_into_basis = "  Prehnite Quartz"
  activity_species = "H2O"
  activity_values = "1"
  equilibrium_species = "Clinoptil-Ca"
  temperature = 200
  interrogation = activity
[]

[UserObjects]
  [./root]
    type = GeochemicalModelRoot
    database_file = "../../../data/moose_geochemdb.json"
    basis_species = "H2O Ca++ Al+++ SiO2(aq) H+ K+"
    equilibrium_minerals = "Clinoptil-Ca Prehnite Quartz"
  [../]
[]


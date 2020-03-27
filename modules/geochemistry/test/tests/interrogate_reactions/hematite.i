# Compute pH and pe values for an equilibrium reaction involving the mineral hematite
[GeochemicalModelInterrogator]
  model_root = root
  swap_out_of_basis = "O2(aq)"
  swap_into_basis = "  e-"
  equilibrium_species = Hematite
  activity_species = 'H2O Fe++'
  activity_values = '1 1E-10'
  interrogation = activity
[]

[UserObjects]
  [./root]
    type = GeochemicalModelRoot
    database_file = "../../../data/moose_geochemdb.json"
    basis_species = "H2O Fe++ H+ O2(aq)"
    equilibrium_minerals = "Hematite"
  [../]
[]


# Output equilibrium reactions for pyrite using different basis components
[GeochemicalModelInterrogator]
  model_root = root
  swap_out_of_basis = "Fe++         Fe(OH)3(ppd) SO4--   O2(aq)"
  swap_into_basis = "  Fe(OH)3(ppd) Fe++         H2S(aq) SO4--"
  equilibrium_species = Pyrite
[]

[UserObjects]
  [./root]
    type = GeochemicalModelRoot
    database_file = "../../../data/moose_geochemdb.json"
    basis_species = "H2O Fe++ SO4-- H+ O2(aq)"
    equilibrium_minerals = "Pyrite Fe(OH)3(ppd)"
  [../]
[]


# Output activity ratios for reactions involving muscovite
[GeochemicalModelInterrogator]
  model_definition = definition
  swap_out_of_basis = "SiO2(aq) Al+++   Quartz   SiO2(aq)  Tridymite SiO2(aq)"
  swap_into_basis = "  Quartz   Maximum SiO2(aq) Tridymite SiO2(aq)  Amrph^silica"
  equilibrium_species = Muscovite
  interrogation = activity
[]

[UserObjects]
  [definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O K+ Al+++ SiO2(aq) H+"
    equilibrium_minerals = "Muscovite Quartz Maximum Tridymite Amrph^silica"
    piecewise_linear_interpolation = true # to get exact logK at 25degC with no best-fit interpolation
  []
[]


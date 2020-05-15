[EquilibriumReactionSolver]
  model_definition = definition
  swap_out_of_basis = "Ca++"
  swap_into_basis = "Gypsum"
  charge_balance_species = "SO4--"
  constraint_species = "H2O Cl- Na+ SO4-- Gypsum"
  constraint_value = "  1.0 1.0E-16 1.0E-16 1E-6  0.5814"
  constraint_meaning = "kg_solvent_water free_molality free_molality moles_bulk_species free_moles_mineral_species"
  verbose = true
  abs_tol = 1E-15
  max_initial_residual = 1E-2
[]

[UserObjects]
  [./definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O Cl- Na+ SO4-- Ca++"
    equilibrium_minerals = "Gypsum"
  [../]
[]

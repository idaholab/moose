[UserObjects]
  [definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O Cl- Na+ SO4--"
    equilibrium_minerals = "Mirabilite"
    piecewise_linear_interpolation = true # for comparison with GWB
  []
[]

[TimeIndependentReactionSolver]
  model_definition = definition
  swap_out_of_basis = "SO4--"
  swap_into_basis = "  Mirabilite"
  charge_balance_species = "Cl-"
  constraint_species = "H2O    Na+ Cl- Mirabilite"
  constraint_value = "  1.0    1.0 1.0 1.0"
  constraint_meaning = "kg_solvent_water bulk_composition bulk_composition free_mineral"
  constraint_unit = "kg moles moles moles"
  ramp_max_ionic_strength_initial = 0 # not needed in this simple problem
  stoichiometric_ionic_str_using_Cl_only = true # for comparison with GWB
  add_aux_pH = false
  mol_cutoff = 1E-5
  abs_tol = 1E-15
[]


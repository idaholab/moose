[UserObjects]
  [definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O Cl- Ca++ H+ HCO3-"
    equilibrium_minerals = "Calcite"
    piecewise_linear_interpolation = true # for comparison with GWB
  []
[]

[TimeIndependentReactionSolver]
  model_definition = definition
  charge_balance_species = "Cl-"
  constraint_species = "H2O              H+       Ca++               Cl-                HCO3-"
  constraint_value = "  1.0              1E-6     1                  1                  1"
  constraint_meaning = "kg_solvent_water activity bulk_composition bulk_composition bulk_composition"
  constraint_unit = "   kg             dimensionless moles           moles              moles"
  ramp_max_ionic_strength_initial = 0 # not needed in this simple problem
  stoichiometric_ionic_str_using_Cl_only = true # for comparison with GWB
  mol_cutoff = 1E-5
  abs_tol = 1E-13
[]

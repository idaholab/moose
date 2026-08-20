# The batch reaction solvers build their own single-node mesh, so "block" is suppressed: setting it
# is an error rather than being silently ignored
[TimeIndependentReactionSolver]
  model_definition = definition
  charge_balance_species = "Cl-"
  constraint_species = "H2O              H+            Cl-"
  constraint_value = "  1.0              -5            1E-5"
  constraint_meaning = "kg_solvent_water log10activity bulk_composition"
  constraint_unit = "   kg               dimensionless moles"
  block = 1
[]

[UserObjects]
  [definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O H+ Cl-"
  []
[]

[TimeIndependentReactionSolver]
  model_definition = definition
  charge_balance_species = "Cl-"
  constraint_species = "H2O              H+           Cl-              Mg++             Na+              SO4--            K+                Br-              Ca++             HCO3-"
# assume that g/l means g/kg(solvent water)
  constraint_value = "  1.0              -7.15         5.500            2.1436           1.8747           0.2852           0.17648          0.02816          0.00499          0.00229"
  constraint_meaning = "kg_solvent_water log10activity bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition"
  constraint_unit = "   kg               dimensionless moles            moles            moles            moles            moles            moles            moles            moles"
  prevent_precipitation = "Halite Anhydrite"
  max_ionic_strength = 10.0
  ramp_max_ionic_strength_initial = 0 # not needed in this simple example
  stoichiometric_ionic_str_using_Cl_only = true # for comparison with GWB
  abs_tol = 1E-12
[]

[UserObjects]
  [definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O H+ Cl- Mg++ Na+ SO4-- K+ Br- Ca++ HCO3-"
    equilibrium_minerals = "Halite Anhydrite"
    piecewise_linear_interpolation = true # for comparison with GWB
  []
[]

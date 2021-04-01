[TimeIndependentReactionSolver]
  model_definition = definition
  temperature = 22
  charge_balance_species = "Cl-" # this means the bulk moles of Cl- will not be exactly as set below
  constraint_species = "H2O              H+            O2(aq)             Cl-              HCO3-            Ca++             Mg++             Na+              K+               Fe++            Fe+++             Mn++             Zn++             SO4--"
  constraint_value = "  1.0              -6.05         0.13438E-3         3.041E-5         0.0295E-3        0.005938E-3      0.01448E-3       0.0018704E-3     0.005115E-3      0.012534E-3     0.0005372E-3      0.005042E-3      0.001897E-3      0.01562E-4"
  constraint_meaning = "kg_solvent_water log10activity free_concentration bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition"
  constraint_unit = "   kg               dimensionless molal              moles            moles            moles            moles            moles            moles            moles            moles            moles            moles            moles"
  max_initial_residual = 1E-2
  ramp_max_ionic_strength_initial = 10
  stoichiometric_ionic_str_using_Cl_only = true # for comparison with GWB
  mol_cutoff = 1E-5
  abs_tol = 1E-15
[]

[UserObjects]
  [definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O H+ Cl- O2(aq) HCO3- Ca++ Mg++ Na+ K+ Fe++ Fe+++ Mn++ Zn++ SO4--"
  []
[]

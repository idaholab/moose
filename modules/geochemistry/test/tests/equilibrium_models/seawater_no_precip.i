[TimeIndependentReactionSolver]
  model_definition = definition
  swap_out_of_basis = "H+     O2(aq)"
  swap_into_basis = "  CO2(g) O2(g)"
  charge_balance_species = "Cl-" # this means the bulk moles of Cl- will not be exactly as set below
  constraint_species = "H2O              CO2(g)    O2(g)         Cl-              Na+              SO4--            Mg++             Ca++             K+               HCO3-            SiO2(aq)"
  constraint_value = "  1.0              0.0003162 0.2           0.566            0.485            0.0292           0.055            0.0106           0.0106           0.00241          0.000103"
  constraint_meaning = "kg_solvent_water fugacity  fugacity      bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition"
  constraint_unit = "   kg          dimensionless  dimensionless moles            moles            moles            moles            moles            moles            moles            moles"
  prevent_precipitation = "Antigorite Tremolite Talc Chrysotile Sepiolite Anthophyllite Dolomite Dolomite-ord Huntite Dolomite-dis Magnesite Calcite Aragonite Quartz"
  ramp_max_ionic_strength_initial = 0 # not needed in this simple problem
  stoichiometric_ionic_str_using_Cl_only = true # for comparison with GWB
  mol_cutoff = 1E-5
  abs_tol = 1E-15
  precision = 7
[]

[UserObjects]
  [definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O H+ Cl- Na+ SO4-- Mg++ Ca++ K+ HCO3- SiO2(aq) O2(aq)"
    equilibrium_minerals = "Antigorite Tremolite Talc Chrysotile Sepiolite Anthophyllite Dolomite Dolomite-ord Huntite Dolomite-dis Magnesite Calcite Aragonite Quartz"
    equilibrium_gases = "O2(g) CO2(g)"
    piecewise_linear_interpolation = true # for comparison with GWB
  []
[]

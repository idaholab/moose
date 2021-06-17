[TimeIndependentReactionSolver]
  model_definition = definition
  swap_out_of_basis = "H+   "
  swap_into_basis = "  MgCO3"
  charge_balance_species = "Cl-" # this means the bulk moles of Cl- will not be exactly as set below
  constraint_species = "H2O              MgCO3            O2(aq)             Cl-              Na+              SO4--            Mg++             Ca++             K+               HCO3-            SiO2(aq)"
# to obtain the constraint on MgCO3: (1) run seawater_no_precip.i to obtain the free molality of MgCO3; (2) then running seawater_no_precip.i with MgCO3 in the basis (in place of H+) with that free molality, to obtain the bulk mole number
  constraint_value = "  1.0              0.0001959        0.2151E-3          0.566            0.485            0.0292           0.055            0.0106           0.0106           0.00241          0.000103"
  constraint_meaning = "kg_solvent_water bulk_composition free_concentration bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition"
  constraint_unit = "   kg               moles            molal              moles            moles            moles            moles            moles            moles            moles            moles"
  prevent_precipitation = "Dolomite-dis Dolomite-ord"
  ramp_max_ionic_strength_initial = 0 # not needed in this simple problem
  stoichiometric_ionic_str_using_Cl_only = true # for comparison with GWB
  mol_cutoff = 1E-5
  abs_tol = 1E-15
[]

[UserObjects]
  [definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O H+ Cl- Na+ SO4-- Mg++ Ca++ K+ HCO3- SiO2(aq) O2(aq)"
    equilibrium_minerals = "Antigorite Tremolite Talc Chrysotile Sepiolite Anthophyllite Dolomite Dolomite-ord Huntite Dolomite-dis Magnesite Calcite Aragonite Quartz"
    piecewise_linear_interpolation = true # for comparison with GWB
  []
[]

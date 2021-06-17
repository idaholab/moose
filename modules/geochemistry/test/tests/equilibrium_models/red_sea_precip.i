# Time-independent model of water from the Red Sea including precipitation
[TimeIndependentReactionSolver]
  model_definition = definition
  swap_out_of_basis = "O2(aq) Ba++"
  swap_into_basis = "Sphalerite Barite"
  charge_balance_species = "Cl-"
  constraint_species = "H2O              H+            Na+              K+               Mg++             Ca++             Cl-              SO4--            HCO3-            Cu+              F-               Fe++            Pb++              Zn++             Sphalerite       Barite"
  constraint_value = "  1.0              -5.6          5.42             0.0643           0.0423           0.173            5.89             0.0118           0.00309          5.50E-06         0.000354         0.00195          4.09E-06         0.000111         5.87E-8          9.772E-6"
  constraint_meaning = "kg_solvent_water log10activity bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition"
  constraint_unit = "   kg               dimensionless moles            moles            moles            moles            moles            moles            moles            moles            moles            moles            moles            moles            moles            moles"
  ramp_max_ionic_strength_initial = 0
  temperature = 60
  stoichiometric_ionic_str_using_Cl_only = true # for comparison with GWB
  mol_cutoff = 1E-7
  abs_tol = 1E-12
[]

[UserObjects]
  [definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O H+ Na+ K+ Mg++ Ca++ Cl- SO4-- HCO3- Cu+ F- Fe++ Pb++ Zn++ O2(aq) Ba++"
    equilibrium_minerals = "Sphalerite Barite Fluorite Chalcocite Bornite Chalcopyrite Pyrite Galena Covellite"
    piecewise_linear_interpolation = true # for comparison with GWB
  []
[]

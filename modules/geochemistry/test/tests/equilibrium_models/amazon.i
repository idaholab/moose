[TimeIndependentReactionSolver]
  model_definition = definition
  charge_balance_species = "Cl-"
  constraint_species = "H2O              H+            O2(aq)             SiO2(aq)         Al+++            Fe++             Ca++             Mg++             Na+              HCO3-            SO4--            Cl-      "
  constraint_value = "  1.0              -6.5          1.813E-4           0.0001165        2.5945E-6        1.0744E-6        0.0001073        4.526E-5         7.83E-5          0.0003114        3.1233E-5        1.383E-4"
  constraint_meaning = "kg_solvent_water log10activity free_concentration bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition"
  constraint_unit = "   kg               dimensionless molal              moles            moles            moles            moles            moles            moles            moles            moles            moles"
  max_initial_residual = 0.1 # this small value is needed so that the charge-balance species is not switched to Ca++ (which, by the way, does not make a huge difference to the result)
  prevent_precipitation = "Nontronit-Ca Nontronit-Mg Nontronit-Na Hematite Kaolinite Beidellit-Ca Beidellit-H Beidellit-Mg Beidellit-Na Pyrophyllite Gibbsite Paragonite Quartz"
  stoichiometric_ionic_str_using_Cl_only = true # for comparison with GWB
  mol_cutoff = 1E-7
  abs_tol = 1E-15
[]

[UserObjects]
  [definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O H+ SiO2(aq) Al+++ Fe++ Ca++ Mg++ Na+ HCO3- SO4-- Cl- O2(aq)"
    equilibrium_minerals = "Nontronit-Ca Nontronit-Mg Nontronit-Na Hematite Kaolinite Beidellit-Ca Beidellit-H Beidellit-Mg Beidellit-Na Pyrophyllite Gibbsite Paragonite Quartz"
    piecewise_linear_interpolation = true # for comparison with GWB
  []
[]

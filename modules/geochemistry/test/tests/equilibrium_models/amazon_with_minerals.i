[TimeIndependentReactionSolver]
  model_definition = definition
  swap_out_of_basis = "Al+++ Fe++"
  swap_into_basis = "Kaolinite Hematite"
  charge_balance_species = "Cl-"
  constraint_species = "H2O              H+            O2(aq)             SiO2(aq)         Kaolinite    Hematite     Ca++             Mg++             Na+              HCO3-            SO4--            Cl-      "
  constraint_value = "  1.0              -6.5          1.813E-4            0.0001165       1E-2         0.033        0.0001073        4.526E-5         7.83E-5          0.0003114        3.1233E-5        1.383E-4"
  constraint_meaning = "kg_solvent_water log10activity free_concentration bulk_composition free_mineral free_mineral bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition"
  constraint_unit = "   kg               dimensionless molal              moles            moles        moles        moles            moles            moles            moles            moles            moles"
  prevent_precipitation = "Nontronit-Ca Nontronit-Mg Nontronit-Na Hematite Kaolinite Beidellit-Ca Beidellit-H Beidellit-Mg Beidellit-Na Pyrophyllite Gibbsite Paragonite Quartz"
  ramp_max_ionic_strength_initial = 0 # not needed in this simple problem
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

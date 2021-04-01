# Sorption onto FerricHydroxide.
[TimeIndependentReactionSolver]
  model_definition = definition
  charge_balance_species = "Cl-"
  swap_out_of_basis = "Fe+++"
  swap_into_basis = "Fe(OH)3(ppd)"
  constraint_species = "H2O              H+            Na+              Cl-              Hg++             Pb++             SO4--            Fe(OH)3(ppd) >(s)FeOH         >(w)FeOH"
  constraint_value = "  1.0              1E-4          10E-3            10E-3            0.1E-3           0.1E-3           0.2E-3           9.3573E-3    4.6786E-5        1.87145E-3"
  constraint_meaning = "kg_solvent_water activity      bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition free_mineral bulk_composition bulk_composition"
  constraint_unit = "   kg               dimensionless moles            moles            moles            moles            moles            moles        moles            moles"
  ramp_max_ionic_strength_initial = 0 # not needed in this simple problem
  stoichiometric_ionic_str_using_Cl_only = true # for comparison with GWB
  mol_cutoff = 1E-10
  abs_tol = 1E-15
[]

[UserObjects]
  [definition]
    type = GeochemicalModelDefinition
    database_file = "../../database/ferric_hydroxide_sorption.json"
    basis_species = "H2O H+ Na+ Cl- Hg++ Pb++ SO4-- Fe+++ >(s)FeOH >(w)FeOH"
    equilibrium_minerals = "Fe(OH)3(ppd)"
    piecewise_linear_interpolation = true # for comparison with GWB
  []
[]

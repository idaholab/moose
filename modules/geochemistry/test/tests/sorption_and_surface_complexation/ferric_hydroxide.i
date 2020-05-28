# Sorption onto FerricHydroxide.
# There is 1 free gram of Fe(OH)3(ppd), which amounts to 9.357E-3 free moles.
# Per mole of Fe(OH)3(ppd) there are 0.005 moles of >(s)FeOH, giving a total of 4.679E-5 moles (bulk composition)
# Per mole of Fe(OH)3(ppd) there are 0.2 moles of >(w)FeOH, giving a total of 1.871E-3 moles (bulk composition)
[EquilibriumReactionSolver]
  model_definition = definition
  swap_out_of_basis = "Fe+++"
  swap_into_basis = "Fe(OH)3(ppd)"
  charge_balance_species = "Cl-"
  constraint_species = "H2O              H+        Na+ Cl- Hg++ Pb++ SO4--          Fe(OH)3(ppd) >(s)FeOH  >(w)FeOH"
  constraint_value = "  1.0              1E-4      10E-3 10E-3 0.1E-3 0.1E-3 0.2E-3 9.3573E-3    4.6786E-5 1.87145E-3"
  constraint_meaning = "kg_solvent_water activity  moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species free_moles_mineral_species moles_bulk_species moles_bulk_species"
  abs_tol = 1E-15
[]

[UserObjects]
  [./definition]
    type = GeochemicalModelDefinition
    database_file = "ferric_hydroxide_sorption.json"
    basis_species = "H2O H+ Na+ Cl- Hg++ Pb++ SO4-- Fe+++ >(s)FeOH >(w)FeOH"
    equilibrium_minerals = "Fe(OH)3(ppd)"
  [../]
[]

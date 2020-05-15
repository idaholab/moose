[EquilibriumReactionSolver]
  model_definition = definition
  charge_balance_species = "Cl-"
  constraint_species = "H2O              H+       Cl- Mg++ Na+ SO4-- K+ Br- Ca++ HCO3-"
# assume that g/l means g/kg(solvent water)
  constraint_value = "  1.0              7.079E-8 5.500239754 2.143591854 1.874744452 0.285245519 0.176478261 0.028158791 0.00499002 0.002294439"
  constraint_meaning = "kg_solvent_water activity moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species"
  max_ionic_strength = 10.0
  prevent_precipitation = "Halite Anhydrite"
[]

[UserObjects]
  [./definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O H+ Cl- Mg++ Na+ SO4-- K+ Br- Ca++ HCO3-"
    equilibrium_minerals = "Halite Anhydrite"
  [../]
[]

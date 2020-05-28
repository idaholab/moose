[EquilibriumReactionSolver]
  model_definition = definition
  swap_out_of_basis = "H+     O2(aq)"
  swap_into_basis = "  CO2(g) O2(g)"
  nernst_swap_out_of_basis = "  CO2(g) O2(g)"
  nernst_swap_into_basis = "    H+"
  charge_balance_species = "Cl-"
  constraint_species = "H2O              CO2(g)       O2(g)    Cl-                Na+                SO4--              Mg++               Ca++               K+                 HCO3-              SiO2(aq)"
  constraint_value = "  1.0              0.0003162278 0.2      0.5656             0.4850             0.02924            0.05501            0.01063            0.010576055        0.002412           0.00010349"
  constraint_meaning = "kg_solvent_water fugacity     fugacity moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species"
[]


[UserObjects]
  [./definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O H+ Cl- Na+ SO4-- Mg++ Ca++ K+ HCO3- SiO2(aq) O2(aq)"
    equilibrium_gases = "O2(g) CO2(g)"
  [../]
[]


[EquilibriumReactionSolver]
  model_definition = definition
  swap_out_of_basis = "H+     O2(aq)"
  swap_into_basis = "  CO2(g) O2(g)"
  charge_balance_species = "Cl-" # this means the bulk moles of Cl- will not be exactly as set below
  constraint_species = "H2O              CO2(g)       O2(g)    Cl-                Na+                SO4--              Mg++               Ca++               K+                 HCO3-              SiO2(aq)"
  constraint_value = "  1.0              0.0003162278 0.2      0.5656             0.4850             0.02924            0.05501            0.01063            0.010576055        0.002412           0.00010349"
  constraint_meaning = "kg_solvent_water fugacity     fugacity moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species"
# Bethke specifies TDS = 35080mg/kg.
# Note that TDS = (mass_non-water) / (mass_solvent_water + mass_non-water),
# so with mass_solvent_water = 1kg, mass_non-water = 0.036355346 kg, and total_mass = 1.036355346 kg
# concentration of Cl- = 19350mg/kg -> moles = 19350E-3 * 1.036355346 / 35.4530 = 0.565635516
  prevent_precipitation = "Antigorite Tremolite Talc Chrysotile Sepiolite Anthophyllite Dolomite Dolomite-ord Huntite Dolomite-dis Magnesite Calcite Aragonite Quartz"
[]


[UserObjects]
  [./definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O H+ Cl- Na+ SO4-- Mg++ Ca++ K+ HCO3- SiO2(aq) O2(aq)"
    equilibrium_minerals = "Antigorite Tremolite Talc Chrysotile Sepiolite Anthophyllite Dolomite Dolomite-ord Huntite Dolomite-dis Magnesite Calcite Aragonite Quartz"
    equilibrium_gases = "O2(g) CO2(g)"
  [../]
[]


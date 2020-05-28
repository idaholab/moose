
[EquilibriumReactionSolver]
  model_definition = definition
  swap_out_of_basis = "O2(aq) Ba++"
  swap_into_basis = "Sphalerite Barite"
  prevent_precipitation = "Sphalerite Barite Fluorite Chalcocite Bornite Chalcopyrite Pyrite Galena Covellite"
  charge_balance_species = "Cl-"
# Approximately TDS = 257000mg/kg
# Note that TDS = (mass_non-water) / (mass_solvent_water + mass_non-water),
# so with mass_solvent_water = 1kg, mass_non-water = 0.345kg kg, and total_mass = 1.345kg
# concentration of Na++ = 92600mg/kg -> moles = 92600E-3 * 1.345 / 22.9898 = 5.42
# etc
  constraint_species = "H2O              H+                 Na+                K+                 Mg++               Ca++               Cl-                SO4--              HCO3-              Cu+                F-                  Fe++                Pb++                Zn++               Sphalerite                 Barite"
  constraint_value = "  1.0              2.512E-6            5.421094523        0.064371691        0.042306677        0.172938108        5.92333512         0.01176952         0.003088074        5.50676E-06        0.000354213         0.001952074         4.09225E-06         0.000111163        1E-11                      0.5E-11"
  constraint_meaning = "kg_solvent_water activity            moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species  moles_bulk_species  moles_bulk_species  moles_bulk_species free_moles_mineral_species free_moles_mineral_species"
  temperature = 60
[]

[UserObjects]
  [./definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O H+ Na+ K+ Mg++ Ca++ Cl- SO4-- HCO3- Cu+ F- Fe++ Pb++ Zn++ O2(aq) Ba++"
    equilibrium_minerals = "Sphalerite Barite Fluorite Chalcocite Bornite Chalcopyrite Pyrite Galena Covellite"
  [../]
[]

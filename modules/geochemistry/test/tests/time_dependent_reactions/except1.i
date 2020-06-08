#Exception: incorrect size of source_species
[TimeDependentReactionSolver]
  model_definition = definition
  geochemistry_reactor_name = reactor
  charge_balance_species = "Cl-"
  constraint_species = "H2O H+ Na+ K+ Ca++ Mg++ Al+++ SiO2(aq) Cl- SO4-- HCO3-"
  constraint_value = "  1.0 1E-5 1E-5 1E-5 1E-5 1E-5 1E-5 1E-5 1E-5 1E-5 1E-5"
  constraint_meaning = "kg_solvent_water activity moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species moles_bulk_species"
  source_species_names = "Al+++"
  source_species_rates = "1 0"
[]

[Executioner]
  type = Steady
[]

[UserObjects]
  [./definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O H+ Na+ K+ Ca++ Mg++ Al+++ SiO2(aq) Cl- SO4-- HCO3-"
  [../]
[]


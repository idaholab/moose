# Finds the equilibrium free-concentration of SiO2(aq) in contact with Quartz at 300degC: this will need to be updated when temperature-dependence has been included in geochemistry
[TimeIndependentReactionSolver]
  model_definition = definition
  geochemistry_reactor_name = reactor
  swap_into_basis = "Quartz"
  swap_out_of_basis = "SiO2(aq)"
  charge_balance_species = "Cl-"
  constraint_species = "H2O H+ Cl- Quartz"
# the amount of free quartz is irrelevant to the equilibrium system, but here 400g is used to make the connection with quartz_deposition.i
  constraint_value = "  1.0 1E-5 1E-5 6.657313"
  constraint_meaning = "kg_solvent_water moles_bulk_species moles_bulk_species free_moles_mineral_species"
  temperature = 300.0
  ramp_max_ionic_strength_initial = 0 # max_ionic_strength in such a simple problem does not need ramping
[]

[UserObjects]
  [./definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O SiO2(aq) H+ Cl-"
    equilibrium_minerals = "Quartz"
  [../]
[]

[EquilibriumReactionSolver]
  model_definition = definition
  temperature = 22
  charge_balance_species = "Cl-" # this means the bulk moles of Cl- will not be exactly as set below
  constraint_species = "H2O H+ O2(aq) Cl- HCO3- Ca++ Mg++ Na+ K+ Fe++ Mn++ Zn++ SO4--"
  constraint_value = "  1.0 0.8913E-6 0.13438E-3 3.041E-5 0.0295E-3 0.005938E-3 0.01448E-3 0.0018704E-3  0.005115E-3 0.01307E-3 0.005042E-3 0.001897E-3 0.01562E-3"
  constraint_meaning = "kg_solvent_water activity free_molality moles_bulk_species  moles_bulk_species  moles_bulk_species  moles_bulk_species  moles_bulk_species  moles_bulk_species  moles_bulk_species  moles_bulk_species  moles_bulk_species  moles_bulk_species"
  abs_tol = 1E-15
  max_initial_residual = 1E-2
[]

[UserObjects]
  [./definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O H+ Cl- O2(aq) HCO3- Ca++ Mg++ Na+ K+ Fe++ Mn++ Zn++ SO4--"
  [../]
[]

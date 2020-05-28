# Langmuir sorption of Selenate
# The reaction is SorbedSelenate = Selenate + SorbingSite with K = 5.4E-6
# Assuming that SorbedSelenate has an activity coefficient of 1 and the activity coefficient of Selenate is also very close to 1,
# molality_{SorbedSelenate} = molality_{Selenate} * molality_{SorbingSite} / K
# The equation for the bulk composition of SorbingSite is
# Moles_{SorbingSite} = n_{w} * (molality_{SorbingSite} + molality_{Sorbed})
# Plugging this into the above equation gives
# molality_{SorbedSelenate} = Moles_{SorbingSite} / n_{w} * r / (1 + r), where r = molality_{Selenate} / K
# The simulation below predicts molality_{SorbedSelenate} given Moles_{SorbingSite}, K and the free molality of Selenate.
# The Moles_{SorbingSite} results from 0.62E-9mol/g(dry soil): there is assumed 500g of dry soil in this simulation (and 1kg of solvent water).
[EquilibriumReactionSolver]
  model_definition = definition
  charge_balance_species = "Na+"
  constraint_species = "H2O              H+        Na+                SeO4--        SorbingSite"
  constraint_value = "  1.0              3.1623E-8 10E-6              5.0E-6       310E-9"
  constraint_meaning = "kg_solvent_water activity  moles_bulk_species free_molality moles_bulk_species"
[]

[UserObjects]
  [./definition]
    type = GeochemicalModelDefinition
    database_file = "selenate_sorption.json"
    basis_species = "H2O H+ Na+ SeO4-- SorbingSite"
  [../]
[]

# Example of biomass death
# In this example, the methanogen does NOT metabolize CH3COO- + H2O -> CH4(aq) + HCO3- because it is not provided with an appropriate GeochemistryKineticRate.  Instead, it simply dies, via:
# d(moles)/dt = -0.5 * moles
# In the database file, the methanogen is provided with a molecular weight 1E9 g/mol, so
# -0.5 * moles = -0.5E-9 * mass
# This is encoded in the rate_biomass_death object below
# Note that the DE is solved using an implicit method, so the solution is
# moles(t + dt) = moles(t) / (1 + 0.5 * dt)
[TimeDependentReactionSolver]
  model_definition = definition
  geochemistry_reactor_name = reactor
  charge_balance_species = "HCO3-"
  constraint_species = "H2O              HCO3-            CH3COO-          CH4(aq)          H+"
  constraint_value = "  1.0              2E-3             1E-6             1E-6             -6"
  constraint_meaning = "kg_solvent_water bulk_composition bulk_composition bulk_composition log10activity"
  constraint_unit = "   kg               moles            moles            moles            dimensionless"
  kinetic_species_name = methanogen
  kinetic_species_initial_value = 1
  kinetic_species_unit = moles
  ramp_max_ionic_strength_initial = 0
  execute_console_output_on = '' # only CSV output for this example
[]

[UserObjects]
  [rate_biomass_death]
    type = GeochemistryKineticRate
    kinetic_species_name = methanogen
    intrinsic_rate_constant = 0.5E-9
    multiply_by_mass = true
    eta = 0
    direction = DEATH
  []
  [definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O H+ CH3COO- CH4(aq) HCO3-"
    kinetic_minerals = methanogen
    kinetic_rate_descriptions = rate_biomass_death
  []
[]

[Executioner]
  type = Transient
  dt = 1E-2
  end_time = 10
[]

[AuxVariables]
  [moles_biomass]
  []
  [transported_acetate]
  []
[]
[AuxKernels]
  [moles_biomass]
    type = GeochemistryQuantityAux
    species = methanogen
    reactor = reactor
    variable = moles_biomass
    quantity = kinetic_moles
  []
  [transported_acetate]
    type = GeochemistryQuantityAux
    species = "CH3COO-"
    reactor = reactor
    variable = transported_acetate
    quantity = transported_moles_in_original_basis
  []
[]
[Postprocessors]
  [moles_biomass]
    type = PointValue
    point = '0 0 0'
    variable = moles_biomass
  []
  [transported_acetate]
    type = PointValue
    point = '0 0 0'
    variable = transported_acetate
  []
[]
[Outputs]
  interval = 100
  csv = true
[]

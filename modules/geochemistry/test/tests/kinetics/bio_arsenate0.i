# The purpose of this is to find the equilibrium constants in the equation
# Lactate- = -2*H2O + 1*CO3-- + 1*CH3COO- - 2*HAsO4-- + 2*As(OH)4-
# The database contains this reaction only: Lactate- = -3O2(aq) + 3HCO3- + 2H+
#
# T=0: Lactate- = -2*H2O + 1*CO3-- + 1*CH3COO- - 2*HAsO4-- + 2*As(OH)4-  .  log10(K) = 6.742
# T=25: Lactate- = -2*H2O + 1*CO3-- + 1*CH3COO- - 2*HAsO4-- + 2*As(OH)4-  .  log10(K) = 26.75
# T=60: Lactate- = -2*H2O + 1*CO3-- + 1*CH3COO- - 2*HAsO4-- + 2*As(OH)4-  .  log10(K) = 49.93
# T=100: Lactate- = -2*H2O + 1*CO3-- + 1*CH3COO- - 2*HAsO4-- + 2*As(OH)4-  .  log10(K) = 71.18
# T=150: Lactate- = -2*H2O + 1*CO3-- + 1*CH3COO- - 2*HAsO4-- + 2*As(OH)4-  .  log10(K) = 92.42
# T=200: Lactate- = -2*H2O + 1*CO3-- + 1*CH3COO- - 2*HAsO4-- + 2*As(OH)4-  .  log10(K) = 109.5
# T=250: Lactate- = -2*H2O + 1*CO3-- + 1*CH3COO- - 2*HAsO4-- + 2*As(OH)4-  .  log10(K) = 123.8
# T=300: Lactate- = -2*H2O + 1*CO3-- + 1*CH3COO- - 2*HAsO4-- + 2*As(OH)4-  .  log10(K) = 136.1
[GeochemicalModelInterrogator]
  model_definition = definition
  swap_into_basis = 'CO3-- HAsO4-- CH3COO-'
  swap_out_of_basis = 'HCO3- H+ O2(aq)'
  temperature = 25.0
[]

[UserObjects]
  [definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O Na+ HCO3- O2(aq) H+ As(OH)4-"
    piecewise_linear_interpolation = true
  []
[]

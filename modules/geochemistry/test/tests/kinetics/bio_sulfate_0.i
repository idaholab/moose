# The purpose of this is to find the equilibrium constants in the equations
# CH3COO- + SO4-- = 2HCO3- + HS-
# CH3COO- + H2O = CH4(aq) + HCO3-
# Results:
# T=0
# CH3COO- = 2*HCO3- - 1*SO4-- + 1*HS-  .  log10(K) = 8.502
# CH3COO- = -1*H2O + 1*HCO3- + 1*CH4(aq)  .  log10(K) = 2.727
# T=25
# CH3COO- = 2*HCO3- - 1*SO4-- + 1*HS-  .  log10(K) = 8.404
# CH3COO- = -1*H2O + 1*HCO3- + 1*CH4(aq)  .  log10(K) = 2.641
# T=60
# CH3COO- = 2*HCO3- - 1*SO4-- + 1*HS-  .  log10(K) = 8.451
# CH3COO- = -1*H2O + 1*HCO3- + 1*CH4(aq)  .  log10(K) = 2.699
# T=100
# CH3COO- = 2*HCO3- - 1*SO4-- + 1*HS-  .  log10(K) = 8.657
# CH3COO- = -1*H2O + 1*HCO3- + 1*CH4(aq)  .  log10(K) = 2.933
# T=150
# CH3COO- = 2*HCO3- - 1*SO4-- + 1*HS-  .  log10(K) = 9.023
# CH3COO- = -1*H2O + 1*HCO3- + 1*CH4(aq)  .  log10(K) = 3.28
# T=200
# CH3COO- = 2*HCO3- - 1*SO4-- + 1*HS-  .  log10(K) = 9.457
# CH3COO- = -1*H2O + 1*HCO3- + 1*CH4(aq)  .  log10(K) = 3.788
# T=250
# CH3COO- = 2*HCO3- - 1*SO4-- + 1*HS-  .  log10(K) = 9.917
# CH3COO- = -1*H2O + 1*HCO3- + 1*CH4(aq)  .  log10(K) = 4.336
# T=300
# CH3COO- = 2*HCO3- - 1*SO4-- + 1*HS-  .  log10(K) = 10.31
# CH3COO- = -1*H2O + 1*HCO3- + 1*CH4(aq)  .  log10(K) = 4.789
[GeochemicalModelInterrogator]
  model_definition = definition
  swap_out_of_basis = 'O2(aq) HS-'
  swap_into_basis = 'HS- CH4(aq)'
  temperature = 300.0
[]

[UserObjects]
  [definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O HCO3- SO4-- H+ O2(aq)"
    piecewise_linear_interpolation = true
  []
[]

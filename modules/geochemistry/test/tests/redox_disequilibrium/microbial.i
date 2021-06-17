[TimeIndependentReactionSolver]
  model_definition = definition
  temperature = 25
  swap_out_of_basis = 'HCO3-  Fe+++'
  swap_into_basis = '  CO2(g) Fe(OH)3(ppd)'
  charge_balance_species = "Cl-"
# TDS = 80.5mg/g (roughly) = (mass_non-water) / (mass_solvent_water + mass_non-water),
# so with mass_solvent_water = 1kg, mass_non-water = 87.6mg and total_mass = 1.0876
  constraint_species = "H2O              H+          CO2(g)        Fe(OH)3(ppd) Cl-              Na+              Ca++             Mg++             SO4--            Fe++             H2(aq)             HS-              O2(aq)             CH4(aq)           NO3-              NO2-              NH4+              CH3COO-"
  constraint_value = "  1.0              1E-6        0.01          0.02914      0.46E-3          0.473E-3         0.407E-3         0.0895E-3        0.3963E-3        3.895E-6         0.004E-6           1.6445E-6        3.399E-6           2.712E-5          3.508E-5          9.456E-6          6.029E-6          5.5526E-6"
  constraint_meaning = "kg_solvent_water activity    fugacity      free_mineral bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition free_concentration bulk_composition free_concentration bulk_composition  bulk_composition  bulk_composition  bulk_composition  bulk_composition"
  constraint_unit = "   kg             dimensionless dimensionless moles        moles            moles            moles            moles            moles            moles            molal              moles            molal              moles             moles             moles             moles             moles"
  ramp_max_ionic_strength_initial = 0 # not needed in this simple problem
  max_initial_residual = 1E-2
  stoichiometric_ionic_str_using_Cl_only = true # for comparison with GWB
  mol_cutoff = 1E-5
  abs_tol = 1E-17
[]

[UserObjects]
  [definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O H+ Cl- Na+ Ca++ Mg++ SO4-- Fe++ H2(aq) HS- O2(aq) Fe+++ HCO3- CH3COO- CH4(aq) NH4+ NO2- NO3-"
    equilibrium_minerals = "Fe(OH)3(ppd)"
    equilibrium_gases = "CO2(g)"
    piecewise_linear_interpolation = true # for comparison with GWB
  []
[]

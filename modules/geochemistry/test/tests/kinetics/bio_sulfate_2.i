# Example of a microbe-catalysed reaction (see Bethke Section 18.5 for further details):
# CH3COO- + SO4-- -> 2HCO3- + HS-
# at pH = 7.2
# at temperature = 25degC
# This file treats the microbe as a kinetic species and all the aqueous components are in equilibrium
[TimeDependentReactionSolver]
  model_definition = definition
  geochemistry_reactor_name = reactor
  swap_into_basis = 'HS-'
  swap_out_of_basis = 'O2(aq)'
  charge_balance_species = "Cl-"
  constraint_species = "H2O              Na+              Ca++             Fe++             Cl-              SO4--            HCO3-            HS-                H+            CH3COO-"
  constraint_value = "  1.0              501E-3           20E-3            2E-3             500E-3           20E-3            2E-3             0.3E-6             -7.2          1E-3"
  constraint_meaning = "kg_solvent_water bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition   log10activity bulk_composition"
  constraint_unit = "   kg               moles            moles            moles            moles            moles            moles            moles              dimensionless moles"
  controlled_activity_name = 'H+'
  controlled_activity_value = 6.30957E-8 # this is pH=7.2
  kinetic_species_name = "sulfate_reducer"
  kinetic_species_initial_value = 0.1
  kinetic_species_unit = mg
  ramp_max_ionic_strength_initial = 0
  stoichiometric_ionic_str_using_Cl_only = true # for comparison with GWB
  execute_console_output_on = ''
  mol_cutoff = 1E-20
  solver_info = true
  evaluate_kinetic_rates_always = true
  prevent_precipitation = 'Pyrite Troilite'
[]

[UserObjects]
  [rate_sulfate_reducer]
    type = GeochemistryKineticRate
    kinetic_species_name = "sulfate_reducer"
    intrinsic_rate_constant = 0.0864 # 1E-9 mol/mg/s = 0.0864 mol/g/day
    multiply_by_mass = true
    promoting_species_names = 'CH3COO-'
    promoting_indices = 1
    promoting_monod_indices = 1
    promoting_half_saturation = 70E-6
    direction = both
    kinetic_biological_efficiency = 4.3E-3
    energy_captured = 45E3
    theta = 0.2
    eta = 1
  []
  [definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O Na+ Ca++ Fe++ Cl- SO4-- HCO3- O2(aq) H+ CH3COO-"
    equilibrium_minerals = "Mackinawite" # other minerals make marginal difference
    kinetic_minerals = "sulfate_reducer"
    kinetic_rate_descriptions = "rate_sulfate_reducer"
    piecewise_linear_interpolation = true # comparison with GWB
  []
[]

[Functions]
  [timestepper]
    type = PiecewiseLinear
    x = '0 10 18  21'
    y = '1E-2 1E-2  1   1'
  []
[]

[Executioner]
  type = Transient
  [TimeStepper]
    type = FunctionDT
    function = timestepper
  []
  end_time = 21
[]

[AuxVariables]
  [moles_acetate]
  []
  [biomass_mg]
  []
[]
[AuxKernels]
  [moles_acetate]
    type = GeochemistryQuantityAux
    species = 'CH3COO-'
    reactor = reactor
    variable = moles_acetate
    quantity = transported_moles_in_original_basis
  []
  [biomass_mg]
    type = GeochemistryQuantityAux
    species = 'sulfate_reducer'
    reactor = reactor
    variable = biomass_mg
    quantity = free_mg
  []
[]
[Postprocessors]
  [moles_acetate]
    type = PointValue
    point = '0 0 0'
    variable = moles_acetate
  []
  [biomass_mg]
    type = PointValue
    point = '0 0 0'
    variable = biomass_mg
  []
[]
[Outputs]
  csv = true
[]

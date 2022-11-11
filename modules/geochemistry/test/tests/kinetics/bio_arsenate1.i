# Example of a microbe-catalysed reaction:
# Lactate- + 2HAsO4-- + 2H2O -> CH3COO- + CO3-- + 2As(OH)4-
# at pH = 9.8
# at temperature = 20degC
# The equation in the database involving lactate is
# Lactate- + 3O2(aq) -> 2H+ + 3HCO3-
# with log10(K) = 231.4 at 20degC
[TimeDependentReactionSolver]
  model_definition = definition
  geochemistry_reactor_name = reactor
  swap_into_basis = 'CO3--'
  swap_out_of_basis = 'HCO3-'
  charge_balance_species = "Cl-"
  constraint_species = "H2O              Na+              CO3--            Lactate-         Cl-              AsO4---          CH3COO-          As(OH)4-         H+"
  constraint_value = "  1.0              1448E-3          24E-3            10E-3            1500E-3          10E-3            1E-6             1E-6             -9.8"
  constraint_meaning = "kg_solvent_water bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition bulk_composition log10activity"
  constraint_unit = "   kg               moles            moles            moles            moles            moles            moles            moles            dimensionless"
  controlled_activity_name = 'H+'
  controlled_activity_value = 1.58489E-10 # this is pH=9.8
  kinetic_species_name = "arsenate_reducer"
  kinetic_species_initial_value = 0.5 # molecular weight of arsenate_reducer = 1, so this is the amount of mmoles too
  kinetic_species_unit = mg
  ramp_max_ionic_strength_initial = 0
  stoichiometric_ionic_str_using_Cl_only = true # for comparison with GWB
  execute_console_output_on = ''
  mol_cutoff = 1E-20
  solver_info = true
  evaluate_kinetic_rates_always = true
  precision = 16
[]

[UserObjects]
  [rate_arsenate_reducer]
    type = GeochemistryKineticRate
    kinetic_species_name = "arsenate_reducer"
    intrinsic_rate_constant = 0.6048 # 7E-9 mol/mg/s = 0.6048 mol/g/day
    promoting_species_names = 'HAsO4--'
    promoting_indices = '1'
    promoting_monod_indices = '1'
    promoting_half_saturation = 10E-6
    multiply_by_mass = true
    direction = dissolution
    kinetic_biological_efficiency = 5
    energy_captured = 125E3
    theta = 0.25
    eta = 1
  []
  [definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O Na+ Cl- HCO3- H+ As(OH)4- Lactate- CH3COO- AsO4---"
    kinetic_redox = "arsenate_reducer"
    kinetic_rate_descriptions = "rate_arsenate_reducer"
  []
[]

[Executioner]
  type = Transient
  dt = 0.01
  end_time = 2
[]

[AuxVariables]
  [moles_acetate]
  []
  [biomass_g]
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
  [biomass_g]
    type = GeochemistryQuantityAux
    species = 'arsenate_reducer'
    reactor = reactor
    variable = biomass_g
    quantity = kinetic_moles # remember molecular weight = 1 g/mol
  []
[]
[Functions]
  [rate]
    type = ParsedFunction
    vars = 'dt reaction_rate_times_dt'
    vals = 'dt reaction_rate_times_dt'
    value = 'reaction_rate_times_dt / dt'
  []
[]
[Postprocessors]
  [moles_acetate]
    type = PointValue
    point = '0 0 0'
    variable = moles_acetate
  []
  [reaction_rate_times_dt]
    type = PointValue
    point = '0 0 0'
    variable = mol_change_arsenate_reducer
    outputs = 'none'
  []
  [dt]
    type = TimestepSize
    outputs = 'none'
  []
  [reaction_rate]
    type = FunctionValuePostprocessor
    function = rate
  []
  [biomass_g]
    type = PointValue
    point = '0 0 0'
    variable = biomass_g
  []
[]
[Outputs]
  csv = true
[]

# Demonstrating that initial precipitates can be dumped and then reactants added
[TimeDependentReactionSolver]
  model_definition = definition
  geochemistry_reactor_name = reactor
  swap_out_of_basis = "HCO3-"
  swap_into_basis = "Calcite"
  charge_balance_species = "Cl-"
  constraint_species = "H2O              Calcite      Ca++             Na+              Cl-              H+"
  constraint_value = "  1.0              10           0.01             0.1              0.11             -8"
  constraint_meaning = "kg_solvent_water free_mineral bulk_composition bulk_composition bulk_composition log10activity"
  constraint_unit = "   kg               cm3          moles            moles            moles            dimensionless"
  ramp_max_ionic_strength_initial = 10
  remove_fixed_activity_name = 'H+'
  remove_fixed_activity_time = 0
  source_species_names = 'HCl'
  source_species_rates = 1E-3
  mode = 1 # in this case, Calcite never re-precipitates, so never need to turn the dump option off
  stoichiometric_ionic_str_using_Cl_only = true # for comparison with GWB
  execute_console_output_on = '' # only CSV output for this test
[]

[Outputs]
  csv = true
  file_base = calcite_dumping_dump
[]

[Postprocessors]
  [cm3_Calcite]
    type = PointValue
    point = '0 0 0'
    variable = 'free_cm3_Calcite'
  []
  [pH]
    type = PointValue
    point = '0 0 0'
    variable = 'pH'
  []
  [molal_CO2aq]
    type = PointValue
    point = '0 0 0'
    variable = 'molal_CO2(aq)'
  []
  [molal_CaCl+]
    type = PointValue
    point = '0 0 0'
    variable = 'molal_CaCl+'
  []
  [molal_HCO3-]
    type = PointValue
    point = '0 0 0'
    variable = 'molal_HCO3-'
  []
  [molal_Ca++]
    type = PointValue
    point = '0 0 0'
    variable = 'molal_Ca++'
  []
  [fugacity_CO2]
    type = PointValue
    point = '0 0 0'
    variable = 'activity_CO2(g)'
  []
[]

[Executioner]
  type = Transient
  dt = 10
  end_time = 100
[]

[UserObjects]
  [definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O H+ Na+ Cl- Ca++ HCO3-"
    equilibrium_minerals = "Calcite"
    equilibrium_gases = "CO2(g)"
    piecewise_linear_interpolation = true # for comparison with GWB
  []
[]


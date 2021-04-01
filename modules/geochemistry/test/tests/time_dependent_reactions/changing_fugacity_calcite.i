#CO2(g) fugacity is changed over time
[TimeDependentReactionSolver]
  model_definition = definition
  geochemistry_reactor_name = reactor
  swap_out_of_basis = "Ca++ H+"
  swap_into_basis = "Calcite CO2(g)"
  charge_balance_species = "HCO3-"
  constraint_species = "H2O              Calcite      CO2(g)        Na+              Cl-              HCO3-"
  constraint_value = "  1.0              0.01354      -3.5          1E-2             1E-2             0"
  constraint_meaning = "kg_solvent_water free_mineral log10fugacity bulk_composition bulk_composition bulk_composition"
  constraint_unit = "   kg               moles        dimensionless moles            moles            moles"
  ramp_max_ionic_strength_initial = 10
  controlled_activity_name = 'CO2(g)'
  controlled_activity_value = fug_co2
  stoichiometric_ionic_str_using_Cl_only = true # for comparison with GWB
  execute_console_output_on = '' # only CSV output required for this example
[]

[AuxVariables]
  [fug_co2]
  []
[]
[AuxKernels]
  [fug_co2]
    type = FunctionAux
    variable = fug_co2
    function = '10^(-3.5*(1 - t))'
    execute_on = timestep_begin # so the correct value is provided to the reactor
  []
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
  [fug_co2]
    type = PointValue
    point = '0 0 0'
    variable = 'activity_CO2(g)'
  []
[]
[Outputs]
  csv = true
[]

[Executioner]
  type = Transient
  dt = 0.1
  end_time = 1
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


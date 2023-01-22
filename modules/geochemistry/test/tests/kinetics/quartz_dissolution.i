# Example of quartz dissolution.
[TimeDependentReactionSolver]
  model_definition = definition
  geochemistry_reactor_name = reactor
  charge_balance_species = "Cl-"
  constraint_species = "H2O              H+               Cl-              SiO2(aq)"
  constraint_value = "  1.0              1E-10            1E-10            1E-9"
  constraint_meaning = "kg_solvent_water bulk_composition bulk_composition free_concentration"
  constraint_unit = "   kg               moles            moles            molal"
  initial_temperature = 100.0
  temperature = 100.0
  kinetic_species_name = Quartz
  kinetic_species_initial_value = 5
  kinetic_species_unit = kg
  ramp_max_ionic_strength_initial = 0 # max_ionic_strength in such a simple problem does not need ramping
  stoichiometric_ionic_str_using_Cl_only = true # for comparison with GWB
  execute_console_output_on = '' # only CSV output for this example
[]

[UserObjects]
  [rate_quartz]
    type = GeochemistryKineticRate
    kinetic_species_name = Quartz
    intrinsic_rate_constant = 1.728E-10 # 2.0E-15mol/s/cm^2 = 1.728E-10mol/day/cm^2
    multiply_by_mass = true
    area_quantity = 1000
  []
  [definition]
    type = GeochemicalModelDefinition
    database_file = "../../../database/moose_geochemdb.json"
    basis_species = "H2O SiO2(aq) H+ Cl-"
    kinetic_minerals = "Quartz"
    kinetic_rate_descriptions = "rate_quartz"
    piecewise_linear_interpolation = true # for comparison with GWB
  []
[]

[Functions]
  [timestepper]
    type = PiecewiseLinear
    x = '0 0.5 3'
    y = '0.01 0.05 0.1'
  []
[]

[Executioner]
  type = Transient
  [TimeStepper]
    type = FunctionDT
    function = timestepper
  []
  end_time = 5.0
[]

[AuxVariables]
  [diss]
  []
[]
[AuxKernels]
  [diss]
    type = ParsedAux
    coupled_variables = moles_Quartz
    expression = '83.216414271 - moles_Quartz'
    variable = diss
  []
[]
[Postprocessors]
  [dissolved_moles]
    type = PointValue
    point = '0 0 0'
    variable = diss
  []
[]
[Outputs]
  csv = true
[]

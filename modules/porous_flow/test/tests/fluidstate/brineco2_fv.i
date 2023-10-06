# Tests correct calculation of properties in PorousFlowBrineCO2 using FV variables

[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 2
  []
[]

[GlobalParams]
  PorousFlowDictator = dictator
  temperature = 30
[]

[Variables]
  [pg]
    type = MooseVariableFVReal
    initial_condition = 20e6
  []
  [z]
    type = MooseVariableFVReal
    initial_condition = 0.2
  []
[]

[AuxVariables]
  [xnacl]
    type = MooseVariableFVReal
    initial_condition = 0.1
  []
  [pressure_gas]
    type = MooseVariableFVReal
  []
  [pressure_water]
    type = MooseVariableFVReal
  []
  [saturation_gas]
    type = MooseVariableFVReal
  []
  [saturation_water]
    type = MooseVariableFVReal
  []
  [density_water]
    type = MooseVariableFVReal
  []
  [density_gas]
    type = MooseVariableFVReal
  []
  [viscosity_water]
    type = MooseVariableFVReal
  []
  [viscosity_gas]
    type = MooseVariableFVReal
  []
  [enthalpy_water]
    type = MooseVariableFVReal
  []
  [enthalpy_gas]
    type = MooseVariableFVReal
  []
  [internal_energy_water]
    type = MooseVariableFVReal
  []
  [internal_energy_gas]
    type = MooseVariableFVReal
  []
  [x0_water]
    type = MooseVariableFVReal
  []
  [x0_gas]
    type = MooseVariableFVReal
  []
  [x1_water]
    type = MooseVariableFVReal
  []
  [x1_gas]
    type = MooseVariableFVReal
  []
[]

[AuxKernels]
  [pressure_water]
    type = ADPorousFlowPropertyAux
    variable = pressure_water
    property = pressure
    phase = 0
    execute_on = 'timestep_end'
  []
  [pressure_gas]
    type = ADPorousFlowPropertyAux
    variable = pressure_gas
    property = pressure
    phase = 1
    execute_on = 'timestep_end'
  []
  [saturation_water]
    type = ADPorousFlowPropertyAux
    variable = saturation_water
    property = saturation
    phase = 0
    execute_on = 'timestep_end'
  []
  [saturation_gas]
    type = ADPorousFlowPropertyAux
    variable = saturation_gas
    property = saturation
    phase = 1
    execute_on = 'timestep_end'
  []
  [density_water]
    type = ADPorousFlowPropertyAux
    variable = density_water
    property = density
    phase = 0
    execute_on = 'timestep_end'
  []
  [density_gas]
    type = ADPorousFlowPropertyAux
    variable = density_gas
    property = density
    phase = 1
    execute_on = 'timestep_end'
  []
  [viscosity_water]
    type = ADPorousFlowPropertyAux
    variable = viscosity_water
    property = viscosity
    phase = 0
    execute_on = 'timestep_end'
  []
  [viscosity_gas]
    type = ADPorousFlowPropertyAux
    variable = viscosity_gas
    property = viscosity
    phase = 1
    execute_on = 'timestep_end'
  []
  [enthalpy_water]
    type = ADPorousFlowPropertyAux
    variable = enthalpy_water
    property = enthalpy
    phase = 0
    execute_on = 'timestep_end'
  []
  [enthalpy_gas]
    type = ADPorousFlowPropertyAux
    variable = enthalpy_gas
    property = enthalpy
    phase = 1
    execute_on = 'timestep_end'
  []
  [internal_energy_water]
    type = ADPorousFlowPropertyAux
    variable = internal_energy_water
    property = internal_energy
    phase = 0
    execute_on = 'timestep_end'
  []
  [internal_energy_gas]
    type = ADPorousFlowPropertyAux
    variable = internal_energy_gas
    property = internal_energy
    phase = 1
    execute_on = 'timestep_end'
  []
  [x1_water]
    type = ADPorousFlowPropertyAux
    variable = x1_water
    property = mass_fraction
    phase = 0
    fluid_component = 1
    execute_on = 'timestep_end'
  []
  [x1_gas]
    type = ADPorousFlowPropertyAux
    variable = x1_gas
    property = mass_fraction
    phase = 1
    fluid_component = 1
    execute_on = 'timestep_end'
  []
  [x0_water]
    type = ADPorousFlowPropertyAux
    variable = x0_water
    property = mass_fraction
    phase = 0
    fluid_component = 0
    execute_on = 'timestep_end'
  []
  [x0_gas]
    type = ADPorousFlowPropertyAux
    variable = x0_gas
    property = mass_fraction
    phase = 1
    fluid_component = 0
    execute_on = 'timestep_end'
  []
[]

[FVKernels]
  [mass0]
    type = FVPorousFlowMassTimeDerivative
    variable = pg
    fluid_component = 0
  []
  [mass1]
    type = FVPorousFlowMassTimeDerivative
    variable = z
    fluid_component = 1
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pg z'
    number_fluid_phases = 2
    number_fluid_components = 2
  []
  [pc]
    type = PorousFlowCapillaryPressureConst
    pc = 0
  []
  [fs]
    type = PorousFlowBrineCO2
    brine_fp = brine
    co2_fp = co2
    capillary_pressure = pc
  []
[]

[FluidProperties]
  [co2]
    type = CO2FluidProperties
  []
  [brine]
    type = BrineFluidProperties
  []
[]

[Materials]
  [temperature]
    type = ADPorousFlowTemperature
  []
  [brineco2]
    type = ADPorousFlowFluidState
    gas_porepressure = pg
    z = z
    temperature_unit = Celsius
    xnacl = xnacl
    capillary_pressure = pc
    fluid_state = fs
  []
  [permeability]
    type = ADPorousFlowPermeabilityConst
    permeability = '1e-12 0 0 0 1e-12 0 0 0 1e-12'
  []
  [relperm0]
    type = ADPorousFlowRelativePermeabilityCorey
    n = 2
    phase = 0
  []
  [relperm1]
    type = ADPorousFlowRelativePermeabilityCorey
    n = 3
    phase = 1
  []
  [porosity]
    type = ADPorousFlowPorosityConst
    porosity = 0.1
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  dt = 1
  end_time = 1
  nl_abs_tol = 1e-12
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Postprocessors]
  [density_water]
    type = ElementIntegralVariablePostprocessor
    variable = density_water
    execute_on = 'timestep_end'
  []
  [density_gas]
    type = ElementIntegralVariablePostprocessor
    variable = density_gas
    execute_on = 'timestep_end'
  []
  [viscosity_water]
    type = ElementIntegralVariablePostprocessor
    variable = viscosity_water
    execute_on = 'timestep_end'
  []
  [viscosity_gas]
    type = ElementIntegralVariablePostprocessor
    variable = viscosity_gas
    execute_on = 'timestep_end'
  []
  [enthalpy_water]
    type = ElementIntegralVariablePostprocessor
    variable = enthalpy_water
    execute_on = 'timestep_end'
  []
  [enthalpy_gas]
    type = ElementIntegralVariablePostprocessor
    variable = enthalpy_gas
    execute_on = 'timestep_end'
  []
  [internal_energy_water]
    type = ElementIntegralVariablePostprocessor
    variable = internal_energy_water
    execute_on = 'timestep_end'
  []
  [internal_energy_gas]
    type = ElementIntegralVariablePostprocessor
    variable = internal_energy_gas
    execute_on = 'timestep_end'
  []
  [x1_water]
    type = ElementIntegralVariablePostprocessor
    variable = x1_water
    execute_on = 'timestep_end'
  []
  [x0_water]
    type = ElementIntegralVariablePostprocessor
    variable = x0_water
    execute_on = 'timestep_end'
  []
  [x1_gas]
    type = ElementIntegralVariablePostprocessor
    variable = x1_gas
    execute_on = 'timestep_end'
  []
  [x0_gas]
    type = ElementIntegralVariablePostprocessor
    variable = x0_gas
    execute_on = 'timestep_end'
  []
  [sg]
    type = ElementIntegralVariablePostprocessor
    variable = saturation_gas
    execute_on = 'timestep_end'
  []
  [sw]
    type = ElementIntegralVariablePostprocessor
    variable = saturation_water
    execute_on = 'timestep_end'
  []
  [pwater]
    type = ElementIntegralVariablePostprocessor
    variable = pressure_water
    execute_on = 'timestep_end'
  []
  [pgas]
    type = ElementIntegralVariablePostprocessor
    variable = pressure_gas
    execute_on = 'timestep_end'
  []
  [x0mass]
    type = FVPorousFlowFluidMass
    fluid_component = 0
    phase = '0 1'
  []
  [x1mass]
    type = FVPorousFlowFluidMass
    fluid_component = 1
    phase = '0 1'
  []
[]

[Outputs]
  csv = true
  file_base = brineco2
  execute_on = 'timestep_end'
  perf_graph = false
[]

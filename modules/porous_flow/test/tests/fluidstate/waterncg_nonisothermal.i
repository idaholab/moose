[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 2
  []
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [pgas]
    initial_condition = 1e6
  []
  [z]
    initial_condition = 0.25
  []
  [temperature]
    initial_condition = 70
  []
[]

[AuxVariables]
  [pressure_gas]
    order = CONSTANT
    family = MONOMIAL
  []
  [pressure_water]
    order = CONSTANT
    family = MONOMIAL
  []
  [saturation_gas]
    order = CONSTANT
    family = MONOMIAL
  []
  [saturation_water]
    order = CONSTANT
    family = MONOMIAL
  []
  [density_water]
    order = CONSTANT
    family = MONOMIAL
  []
  [density_gas]
    order = CONSTANT
    family = MONOMIAL
  []
  [viscosity_water]
    order = CONSTANT
    family = MONOMIAL
  []
  [viscosity_gas]
    order = CONSTANT
    family = MONOMIAL
  []
  [enthalpy_water]
    order = CONSTANT
    family = MONOMIAL
  []
  [enthalpy_gas]
    order = CONSTANT
    family = MONOMIAL
  []
  [internal_energy_water]
    order = CONSTANT
    family = MONOMIAL
  []
  [internal_energy_gas]
    order = CONSTANT
    family = MONOMIAL
  []
  [x0_water]
    order = CONSTANT
    family = MONOMIAL
  []
  [x0_gas]
    order = CONSTANT
    family = MONOMIAL
  []
  [x1_water]
    order = CONSTANT
    family = MONOMIAL
  []
  [x1_gas]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [pressure_water]
    type = PorousFlowPropertyAux
    variable = pressure_water
    property = pressure
    phase = 0
    execute_on = timestep_end
  []
  [pressure_gas]
    type = PorousFlowPropertyAux
    variable = pressure_gas
    property = pressure
    phase = 1
    execute_on = timestep_end
  []
  [saturation_water]
    type = PorousFlowPropertyAux
    variable = saturation_water
    property = saturation
    phase = 0
    execute_on = timestep_end
  []
  [saturation_gas]
    type = PorousFlowPropertyAux
    variable = saturation_gas
    property = saturation
    phase = 1
    execute_on = timestep_end
  []
  [density_water]
    type = PorousFlowPropertyAux
    variable = density_water
    property = density
    phase = 0
    execute_on = timestep_end
  []
  [density_gas]
    type = PorousFlowPropertyAux
    variable = density_gas
    property = density
    phase = 1
    execute_on = timestep_end
  []
  [viscosity_water]
    type = PorousFlowPropertyAux
    variable = viscosity_water
    property = viscosity
    phase = 0
    execute_on = timestep_end
  []
  [viscosity_gas]
    type = PorousFlowPropertyAux
    variable = viscosity_gas
    property = viscosity
    phase = 1
    execute_on = timestep_end
  []
  [enthalpy_water]
    type = PorousFlowPropertyAux
    variable = enthalpy_water
    property = enthalpy
    phase = 0
    execute_on = timestep_end
  []
  [enthalpy_gas]
    type = PorousFlowPropertyAux
    variable = enthalpy_gas
    property = enthalpy
    phase = 1
    execute_on = timestep_end
  []
  [internal_energy_water]
    type = PorousFlowPropertyAux
    variable = internal_energy_water
    property = internal_energy
    phase = 0
    execute_on = timestep_end
  []
  [internal_energy_gas]
    type = PorousFlowPropertyAux
    variable = internal_energy_gas
    property = internal_energy
    phase = 1
    execute_on = timestep_end
  []
  [x1_water]
    type = PorousFlowPropertyAux
    variable = x1_water
    property = mass_fraction
    phase = 0
    fluid_component = 1
    execute_on = timestep_end
  []
  [x1_gas]
    type = PorousFlowPropertyAux
    variable = x1_gas
    property = mass_fraction
    phase = 1
    fluid_component = 1
    execute_on = timestep_end
  []
  [x0_water]
    type = PorousFlowPropertyAux
    variable = x0_water
    property = mass_fraction
    phase = 0
    fluid_component = 0
    execute_on = timestep_end
  []
  [x0_gas]
    type = PorousFlowPropertyAux
    variable = x0_gas
    property = mass_fraction
    phase = 1
    fluid_component = 0
    execute_on = timestep_end
  []
[]

[Kernels]
  [mass0]
    type = PorousFlowMassTimeDerivative
    variable = pgas
    fluid_component = 0
  []
  [mass1]
    type = PorousFlowMassTimeDerivative
    variable = z
    fluid_component = 1
  []
  [heat]
    type = TimeDerivative
    variable = temperature
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pgas z '
    number_fluid_phases = 2
    number_fluid_components = 2
  []
  [pc]
    type = PorousFlowCapillaryPressureConst
    pc = 0
  []
  [fs]
    type = PorousFlowWaterNCG
    water_fp = water
    gas_fp = co2
    capillary_pressure = pc
  []
[]

[FluidProperties]
  [co2]
    type = CO2FluidProperties
  []
  [water]
    type = Water97FluidProperties
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
    temperature = temperature
  []
  [waterncg]
    type = PorousFlowFluidState
    gas_porepressure = pgas
    z = z
    temperature = temperature
    temperature_unit = Celsius
    capillary_pressure = pc
    fluid_state = fs
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1e-12 0 0 0 1e-12 0 0 0 1e-12'
  []
  [relperm0]
    type = PorousFlowRelativePermeabilityCorey
    n = 2
    phase = 0
  []
  [relperm1]
    type = PorousFlowRelativePermeabilityCorey
    n = 3
    phase = 1
  []
  [porosity]
    type = PorousFlowPorosityConst
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
  []
  [density_gas]
    type = ElementIntegralVariablePostprocessor
    variable = density_gas
  []
  [viscosity_water]
    type = ElementIntegralVariablePostprocessor
    variable = viscosity_water
  []
  [viscosity_gas]
    type = ElementIntegralVariablePostprocessor
    variable = viscosity_gas
  []
  [enthalpy_water]
    type = ElementIntegralVariablePostprocessor
    variable = enthalpy_water
  []
  [enthalpy_gas]
    type = ElementIntegralVariablePostprocessor
    variable = enthalpy_gas
  []
  [internal_energy_water]
    type = ElementIntegralVariablePostprocessor
    variable = internal_energy_water
  []
  [internal_energy_gas]
    type = ElementIntegralVariablePostprocessor
    variable = internal_energy_gas
  []
  [x0_water]
    type = ElementIntegralVariablePostprocessor
    variable = x0_water
  []
  [x1_gas]
    type = ElementIntegralVariablePostprocessor
    variable = x1_gas
  []
  [x0_gas]
    type = ElementIntegralVariablePostprocessor
    variable = x0_gas
  []
  [sg]
    type = ElementIntegralVariablePostprocessor
    variable = saturation_gas
  []
  [sw]
    type = ElementIntegralVariablePostprocessor
    variable = saturation_water
  []
  [pwater]
    type = ElementIntegralVariablePostprocessor
    variable = pressure_water
  []
  [pgas]
    type = ElementIntegralVariablePostprocessor
    variable = pressure_gas
  []
  [x0mass]
    type = PorousFlowFluidMass
    fluid_component = 0
    phase = '0 1'
  []
  [x1mass]
    type = PorousFlowFluidMass
    fluid_component = 1
    phase = '0 1'
  []
[]

[Outputs]
  csv = true
  execute_on = timestep_end
[]

# Injection of supercritical CO2 into a single brine saturated cell. The CO2 initially fully
# dissolves into the brine, increasing its density slightly. After a few time steps,
# the brine is saturated with CO2, and subsequently a supercritical gas phase of CO2 saturated
# with a small amount of H2O is formed. Salt is included as a nonlinear variable.

[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[GlobalParams]
  PorousFlowDictator = dictator
  temperature = 30
[]

[Variables]
  [pgas]
    initial_condition = 20e6
  []
  [z]
  []
  [xnacl]
    initial_condition = 0.1
  []
[]

[DiracKernels]
  [source]
    type = PorousFlowSquarePulsePointSource
    variable = z
    point = '0.5 0.5 0'
    mass_flux = 2
  []
[]

[BCs]
  [left]
    type = DirichletBC
    value = 20e6
    variable = pgas
    boundary = left
  []
  [right]
    type = DirichletBC
    value = 20e6
    variable = pgas
    boundary = right
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
    execute_on = 'initial timestep_end'
  []
  [pressure_gas]
    type = PorousFlowPropertyAux
    variable = pressure_gas
    property = pressure
    phase = 1
    execute_on = 'initial timestep_end'
  []
  [saturation_water]
    type = PorousFlowPropertyAux
    variable = saturation_water
    property = saturation
    phase = 0
    execute_on = 'initial timestep_end'
  []
  [saturation_gas]
    type = PorousFlowPropertyAux
    variable = saturation_gas
    property = saturation
    phase = 1
    execute_on = 'initial timestep_end'
  []
  [density_water]
    type = PorousFlowPropertyAux
    variable = density_water
    property = density
    phase = 0
    execute_on = 'initial timestep_end'
  []
  [density_gas]
    type = PorousFlowPropertyAux
    variable = density_gas
    property = density
    phase = 1
    execute_on = 'initial timestep_end'
  []
  [viscosity_water]
    type = PorousFlowPropertyAux
    variable = viscosity_water
    property = viscosity
    phase = 0
    execute_on = 'initial timestep_end'
  []
  [viscosity_gas]
    type = PorousFlowPropertyAux
    variable = viscosity_gas
    property = viscosity
    phase = 1
    execute_on = 'initial timestep_end'
  []
  [x1_water]
    type = PorousFlowPropertyAux
    variable = x1_water
    property = mass_fraction
    phase = 0
    fluid_component = 1
    execute_on = 'initial timestep_end'
  []
  [x1_gas]
    type = PorousFlowPropertyAux
    variable = x1_gas
    property = mass_fraction
    phase = 1
    fluid_component = 1
    execute_on = 'initial timestep_end'
  []
  [x0_water]
    type = PorousFlowPropertyAux
    variable = x0_water
    property = mass_fraction
    phase = 0
    fluid_component = 0
    execute_on = 'initial timestep_end'
  []
  [x0_gas]
    type = PorousFlowPropertyAux
    variable = x0_gas
    property = mass_fraction
    phase = 1
    fluid_component = 0
    execute_on = 'initial timestep_end'
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
  [mass2]
    type = PorousFlowMassTimeDerivative
    variable = xnacl
    fluid_component = 2
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pgas z xnacl'
    number_fluid_phases = 2
    number_fluid_components = 3
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
    type = PorousFlowTemperature
  []
  [brineco2]
    type = PorousFlowFluidState
    gas_porepressure = pgas
    z = z
    temperature_unit = Celsius
    xnacl = xnacl
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
  end_time = 10
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
    execute_on = 'initial timestep_end'
  []
  [density_gas]
    type = ElementIntegralVariablePostprocessor
    variable = density_gas
    execute_on = 'initial timestep_end'
  []
  [viscosity_water]
    type = ElementIntegralVariablePostprocessor
    variable = viscosity_water
    execute_on = 'initial timestep_end'
  []
  [viscosity_gas]
    type = ElementIntegralVariablePostprocessor
    variable = viscosity_gas
    execute_on = 'initial timestep_end'
  []
  [x1_water]
    type = ElementIntegralVariablePostprocessor
    variable = x1_water
    execute_on = 'initial timestep_end'
  []
  [x0_water]
    type = ElementIntegralVariablePostprocessor
    variable = x0_water
    execute_on = 'initial timestep_end'
  []
  [x1_gas]
    type = ElementIntegralVariablePostprocessor
    variable = x1_gas
    execute_on = 'initial timestep_end'
  []
  [x0_gas]
    type = ElementIntegralVariablePostprocessor
    variable = x0_gas
    execute_on = 'initial timestep_end'
  []
  [sg]
    type = ElementIntegralVariablePostprocessor
    variable = saturation_gas
    execute_on = 'initial timestep_end'
  []
  [sw]
    type = ElementIntegralVariablePostprocessor
    variable = saturation_water
    execute_on = 'initial timestep_end'
  []
  [pwater]
    type = ElementIntegralVariablePostprocessor
    variable = pressure_water
    execute_on = 'initial timestep_end'
  []
  [pgas]
    type = ElementIntegralVariablePostprocessor
    variable = pressure_gas
    execute_on = 'initial timestep_end'
  []
  [xnacl]
    type = ElementIntegralVariablePostprocessor
    variable = xnacl
    execute_on = 'initial timestep_end'
  []
  [x0mass]
    type = PorousFlowFluidMass
    fluid_component = 0
    phase = '0 1'
    execute_on = 'initial timestep_end'
  []
  [x1mass]
    type = PorousFlowFluidMass
    fluid_component = 1
    phase = '0 1'
    execute_on = 'initial timestep_end'
  []
  [x2mass]
    type = PorousFlowFluidMass
    fluid_component = 2
    phase = '0 1'
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  csv = true
  file_base = brineco2_2
  execute_on = 'initial timestep_end'
  perf_graph = true
[]

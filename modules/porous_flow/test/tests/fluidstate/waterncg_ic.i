# Tests correct calculation of z (total mass fraction of NCG summed over all
# phases) using the PorousFlowFluidStateIC initial condition. Once z is
# calculated by the initial condition, the thermophysical properties are calculated
# and the resulting gas saturation should be equal to that given in the intial condition

[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[GlobalParams]
  PorousFlowDictator = dictator
  temperature_unit = Celsius
[]

[Variables]
  [pgas]
    initial_condition = 1e6
  []
  [z]
  []
[]

[ICs]
  [z]
    type = PorousFlowFluidStateIC
    saturation = 0.5
    gas_porepressure = pgas
    temperature = 50
    variable = z
    fluid_state = fs
  []
[]

[AuxVariables]
  [saturation_gas]
    order = CONSTANT
    family = MONOMIAL
  []
  [saturation_water]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
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
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pgas z'
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
    temperature = 50
  []
  [waterncg]
    type = PorousFlowFluidState
    gas_porepressure = pgas
    z = z
    fluid_state = fs
    capillary_pressure = pc
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
  [z]
    type = ElementIntegralVariablePostprocessor
    variable = z
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  csv = true
[]

# Tests correct calculation of properties derivatives in PorousFlowWaterNCG
# for nonisothermal two phase conditions

[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 0 0'
[]

[Variables]
  [pgas]
  []
  [z]
  []
  [temperature]
  []
[]

[ICs]
  [pgas]
    type = RandomIC
    min = 1e5
    max = 5e5
    variable = pgas
  []
  [z]
    type = RandomIC
    min = 0.01
    max = 0.06
    variable = z
  []
  [temperature]
    type = RandomIC
    min = 20
    max = 80
    variable = temperature
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
  [adv0]
    type = PorousFlowAdvectiveFlux
    variable = pgas
    fluid_component = 0
  []
  [adv1]
    type = PorousFlowAdvectiveFlux
    variable = z
    fluid_component = 1
  []
  [energy]
    type = PorousFlowEnergyTimeDerivative
    variable = temperature
  []
  [heat]
    type = PorousFlowHeatAdvection
    variable = temperature
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pgas z temperature'
    number_fluid_phases = 2
    number_fluid_components = 2
  []
  [pc]
    type = PorousFlowCapillaryPressureVG
    m = 0.5
    alpha = 1e1
    pc_max = 1e4
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
    n = 2
    phase = 1
  []
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.1
  []
  [rock_heat]
    type = PorousFlowMatrixInternalEnergy
    specific_heat_capacity = 1000
    density = 2500
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

[AuxVariables]
  [sgas]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [sgas]
    type = PorousFlowPropertyAux
    property = saturation
    phase = 1
    variable = sgas
  []
[]

[Postprocessors]
  [sgas_min]
    type = ElementExtremeValue
    variable = sgas
    value_type = min
  []
  [sgas_max]
    type = ElementExtremeValue
    variable = sgas
    value_type = max
  []
[]

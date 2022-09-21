# Tests correct calculation of properties derivatives in PorousFlowFluidState
# for conditions that are appropriate for two phases

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 0 0'
[]

[AuxVariables]
  [xnacl]
    initial_condition = 0.05
  []
[]

[Variables]
  [pgas]
  []
  [zi]
  []
[]

[ICs]
  [pgas]
    type = RandomIC
    min = 1e6
    max = 4e6
    variable = pgas
    seed = 1
  []
  [z]
    type = RandomIC
    min = 0.2
    max = 0.8
    variable = zi
    seed = 2
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
    variable = zi
    fluid_component = 1
  []
  [adv0]
    type = PorousFlowAdvectiveFlux
    variable = pgas
    fluid_component = 0
  []
  [adv1]
    type = PorousFlowAdvectiveFlux
    variable = zi
    fluid_component = 1
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pgas zi'
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
  [water]
    type = Water97FluidProperties
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
    temperature = 50
  []
  [brineco2]
    type = PorousFlowFluidState
    gas_porepressure = pgas
    z = zi
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

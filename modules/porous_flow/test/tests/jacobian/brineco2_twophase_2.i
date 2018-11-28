# Tests correct calculation of properties derivatives in PorousFlowFluidState
# for conditions for two phases, including salt as a nonlinear variable

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

[Variables]
  [./pgas]
  [../]
  [./zi]
  [../]
  [./xnacl]
  [../]
[]

[ICs]
  [./pgas]
    type = RandomIC
    min = 1e6
    max = 4e6
    variable = pgas
    seed = 1
  [../]
  [./z]
    type = RandomIC
    min = 0.2
    max = 0.8
    variable = zi
    seed = 1
  [../]
  [./xnacl]
    type = RandomIC
    min = 0.01
    max = 0.15
    variable = xnacl
    seed = 1
  [../]
[]

[Kernels]
  [./mass0]
    type = PorousFlowMassTimeDerivative
    variable = pgas
    fluid_component = 0
  [../]
  [./mass1]
    type = PorousFlowMassTimeDerivative
    variable = zi
    fluid_component = 1
  [../]
  [./mass2]
    type = PorousFlowMassTimeDerivative
    variable = xnacl
    fluid_component = 2
  [../]
  [./adv0]
    type = PorousFlowAdvectiveFlux
    variable = pgas
    fluid_component = 0
  [../]
  [./adv1]
    type = PorousFlowAdvectiveFlux
    variable = zi
    fluid_component = 1
  [../]
  [./adv2]
    type = PorousFlowAdvectiveFlux
    variable = xnacl
    fluid_component = 2
  [../]
[]

[UserObjects]
  [./dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'pgas zi xnacl'
    number_fluid_phases = 2
    number_fluid_components = 3
  [../]
  [./pc]
    type = PorousFlowCapillaryPressureVG
    m = 0.5
    alpha = 1
    pc_max = 1e3
  [../]
  [./fs]
    type = PorousFlowBrineCO2
    brine_fp = brine
    co2_fp = co2
    capillary_pressure = pc
  [../]
[]

[Modules]
  [./FluidProperties]
    [./co2]
      type = CO2FluidProperties
    [../]
    [./brine]
      type = BrineFluidProperties
    [../]
  [../]
[]

[Materials]
  [./temperature]
    type = PorousFlowTemperature
    temperature = 50
  [../]
  [./temperature_nodal]
    type = PorousFlowTemperature
    temperature = 50
    at_nodes = true
  [../]
  [./brineco2]
    type = PorousFlowFluidState
    gas_porepressure = pgas
    z = zi
    at_nodes = true
    temperature_unit = Celsius
    xnacl = xnacl
    capillary_pressure = pc
    fluid_state = fs
  [../]
  [./brineco2_qp]
    type = PorousFlowFluidState
    gas_porepressure = pgas
    z = zi
    temperature_unit = Celsius
    xnacl = xnacl
    capillary_pressure = pc
    fluid_state = fs
  [../]
  [./permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1e-12 0 0 0 1e-12 0 0 0 1e-12'
  [../]
  [./relperm0]
    type = PorousFlowRelativePermeabilityCorey
    n = 2
    phase = 0
    at_nodes = true
  [../]
  [./relperm1]
    type = PorousFlowRelativePermeabilityCorey
    n = 3
    phase = 1
    at_nodes = true
  [../]
  [./porosity]
    type = PorousFlowPorosityConst
    porosity = 0.1
    at_nodes = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  dt = 1
  end_time = 1
  nl_abs_tol = 1e-12
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

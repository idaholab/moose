# 1phase, using fully-saturated, fully-upwinded version, heat advection
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  xmin = 0
  xmax = 1
  ny = 1
  ymin = 0
  ymax = 1
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[Variables]
  [temp]
  []
  [pp]
  []
[]

[ICs]
  [temp]
    type = RandomIC
    variable = temp
    max = 1.0
    min = 0.0
  []
  [pp]
    type = RandomIC
    variable = pp
    max = 0.0
    min = -1.0
  []
[]

[Kernels]
  [pp]
    type = TimeDerivative
    variable = pp
  []
  [heat_advection]
    type = PorousFlowFullySaturatedUpwindHeatAdvection
    variable = temp
    gravity = '1 2 3'
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'temp pp'
    number_fluid_phases = 1
    number_fluid_components = 1
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 0.5
    density0 = 1.1
    thermal_expansion = 1
    viscosity = 1
    cv = 1.1
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
    temperature = temp
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1 0 0 0 2 0 0 0 3'
  []
  [PS]
    type = PorousFlow1PhaseFullySaturated
    porepressure = pp
  []
  [simple_fluid]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  []
[]

[Preconditioning]
  active = check
  [check]
    type = SMP
    full = true
    petsc_options_iname = '-snes_type'
    petsc_options_value = 'test'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1
  end_time = 1
[]

[Outputs]
  exodus = false
[]

# Exception testing: cannot use PorousFlowFullySaturatedUpwindHeatAdvection with != 1 phase
[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 0 0'
[]

[Variables]
  [ppwater]
  []
  [ppgas]
  []
  [temp]
  []
[]

[Kernels]
  [dummy1]
    type = Diffusion
    variable = ppwater
  []
  [dummy2]
    type = Diffusion
    variable = ppgas
  []
  [advection]
    type = PorousFlowFullySaturatedUpwindHeatAdvection
    variable = temp
  []
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    porous_flow_vars = 'ppwater ppgas temp'
    number_fluid_phases = 2
    number_fluid_components = 1
  []
  [pc]
    type = PorousFlowCapillaryPressureConst
  []
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
  []
[]

[Materials]
  [temperature]
    type = PorousFlowTemperature
    temperature = temp
  []
  [ppss]
    type = PorousFlow2PhasePP
    phase0_porepressure = ppwater
    phase1_porepressure = ppgas
    capillary_pressure = pc
  []
  [massfrac]
    type = PorousFlowMassFraction
  []
  [simple_fluid0]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  []
  [simple_fluid1]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 1
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1.1 0 0 0 2 0 0 0 3'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1
  num_steps = 1
[]

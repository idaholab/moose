# Exception test: S_gr_max is too large
[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[GlobalParams]
  PorousFlowDictator = dictator
[]

[UserObjects]
  [dictator]
    type = PorousFlowDictator
    number_fluid_phases = 1
    number_fluid_components = 1
    porous_flow_vars = 'pp'
  []
  [pc]
    type = PorousFlowCapillaryPressureConst
  []
[]

[Variables]
  [pp]
  []
[]

[Kernels]
  [mass_conservation]
    type = PorousFlowMassTimeDerivative
    variable = pp
  []
[]


[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
  []
[]

[Materials]
  [porosity]
    type = PorousFlowPorosityConst
    porosity = 0.1
  []
  [temperature]
    type = PorousFlowTemperature
  []
  [massfrac]
    type = PorousFlowMassFraction
  []
  [simple_fluid]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  []
  [pc_calculator]
    type = PorousFlow1PhaseP
    capillary_pressure = pc
    porepressure = pp
  []
  [hys_order_material]
    type = PorousFlowHysteresisOrder
  []
  [relperm_material]
    type = PorousFlowHystereticRelativePermeabilityLiquid
    phase = 0
    S_lr = 0.1
    S_gr_max = 0.9
    m = 0.9
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 1
  end_time = 1
[]

[AuxVariables]
  [hys_order]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [hys_order]
    type = PorousFlowPropertyAux
    variable = hys_order
    property = hysteresis_order
  []
[]

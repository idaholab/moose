# Simple example of a 1-phase situation with hysteretic capillary pressure.  Water is removed and added to the system in order to observe the hysteresis.  A PorousFlowSink is used to remove and add water.  This input file is analogous to 1phase.i, but uses PorousFlowSink instead of PorousFlowPointSourceFromPostprocessor to remove and add water
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
[]

[Variables]
  [pp]
    initial_condition = 0
  []
[]

[Kernels]
  [mass_conservation]
    type = PorousFlowMassTimeDerivative
    variable = pp
  []
[]

[BCs]
  [pump]
    type = PorousFlowSink
    flux_function = '-0.5 * if(t <= 9, -10, 10)'
    boundary = 'left right'
    variable = pp
  []
[]

[AuxVariables]
  [sat]
    family = MONOMIAL
    order = CONSTANT
  []
  [hys_order]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [sat]
    type = PorousFlowPropertyAux
    variable = sat
    property = saturation
  []
  [hys_order]
    type = PorousFlowPropertyAux
    variable = hys_order
    property = hysteresis_order
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
    temperature = 20
  []
  [massfrac]
    type = PorousFlowMassFraction
  []
  [simple_fluid]
    type = PorousFlowSingleComponentFluid
    fp = simple_fluid
    phase = 0
  []
  [hys_order_material]
    type = PorousFlowHysteresisOrder
  []
  [pc_calculator]
    type = PorousFlow1PhaseHysP
    alpha_d = 10.0
    alpha_w = 7.0
    n_d = 1.5
    n_w = 1.9
    S_l_min = 0.1
    S_lr = 0.2
    S_gr_max = 0.3
    Pc_max = 12.0
    high_ratio = 0.9
    low_extension_type = quadratic
    high_extension_type = power
    porepressure = pp
  []
[]

[Postprocessors]
  [flux]
    type = FunctionValuePostprocessor
    function = 'if(t <= 9, -10, 10)'
  []
  [hys_order]
    type = PointValue
    point = '0 0 0'
    variable = hys_order
  []
  [sat]
    type = PointValue
    point = '0 0 0'
    variable = sat
  []
  [pp]
    type = PointValue
    point = '0 0 0'
    variable = pp
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 0.5
  end_time = 19
  nl_abs_tol = 1E-10
[]

[Outputs]
  csv = true
[]

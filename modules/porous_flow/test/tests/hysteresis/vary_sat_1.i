# The saturation is varied with time and the capillary pressure is computed
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
    porous_flow_vars = ''
  []
[]

[Variables]
  [dummy]
  []
[]

[Kernels]
  [dummy]
    type = TimeDerivative
    variable = dummy
  []
[]

[AuxVariables]
  [sat]
    initial_condition = 1
  []
  [hys_order]
    family = MONOMIAL
    order = CONSTANT
  []
  [pc]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [sat_aux]
    type = FunctionAux
    variable = sat
    function = '1 - t'
  []
  [hys_order]
    type = PorousFlowPropertyAux
    variable = hys_order
    property = hysteresis_order
  []
  [pc]
    type = PorousFlowPropertyAux
    variable = pc
    property = hysteretic_info
  []
[]

[Materials]
  [hys_order]
    type = PorousFlowHysteresisOrder
  []
  [pc_calculator]
    type = PorousFlowHystereticInfo
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
    sat_var = sat
  []
[]

[Postprocessors]
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
  [pc]
    type = PointValue
    point = '0 0 0'
    variable = pc
  []
[]

[Executioner]
  type = Transient
  solve_type = Linear
  dt = 0.1
  end_time = 1
[]

[Outputs]
  csv = true
[]

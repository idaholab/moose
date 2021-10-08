# 1-phase hysteresis.  Saturation calculation.  Primary drying curve with low_extension_type = exponential
# When comparing the results with a by-hand computation, remember the MOOSE results are averaged over an element
[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = 0
    xmax = 10
    nx = 100
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
    porous_flow_vars = pp
  []
[]

[Variables]
  [pp]
  []
[]

[ICs]
  [pp]
    type = FunctionIC
    variable = pp
    function = '1 - 2 * x'
  []
[]

[BCs]
  [pp]
    type = FunctionDirichletBC
    variable = pp
    function = '1 - 2 * x'
    boundary = 'left right'
  []
[]

[Kernels]
  [dummy]
    type = Diffusion
    variable = pp
  []
[]

[Materials]
  [hys_order]
    type = PorousFlowHysteresisOrder
  []
  [saturation_calculator]
    type = PorousFlow1PhaseHysP
    alpha_d = 10.0
    alpha_w = 10.0
    n_d = 1.1
    n_w = 1.9
    S_l_min = 0.1
    S_lr = 0.2
    S_gr_max = 0.3
    Pc_max = 7.0
    low_extension_type = exponential
    porepressure = pp
  []
[]

[AuxVariables]
  [hys_order]
    family = MONOMIAL
    order = CONSTANT
  []
  [saturation]
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
  [saturation]
    type = PorousFlowPropertyAux
    variable = saturation
    property = saturation
    phase = 0
  []
[]

[VectorPostprocessors]
  [sat]
    type = LineValueSampler
    warn_discontinuous_face_values = false
    start_point = '0.5 0 0'
    end_point = '9.5 0 0'
    num_points = 10
    sort_by = x
    variable = 'saturation pp'
  []
[]

[Executioner]
  type = Transient
  solve_type = Linear
  dt = 1
  end_time = 1
[]

[Outputs]
  csv = true
[]

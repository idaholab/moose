# Capillary-pressure calculation.  Primary drying curve with low_extension_type = none
# When comparing the results with a by-hand computation, remember the MOOSE results are averaged over an element
[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = 0
    xmax = 1
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
    porous_flow_vars = ''
  []
[]

[Variables]
  [sat]
  []
[]

[ICs]
  [sat]
    type = FunctionIC
    variable = sat
    function = 'x'
  []
[]

[BCs]
  [sat]
    type = FunctionDirichletBC
    variable = sat
    function = 'x'
    boundary = 'left right'
  []
[]


[Kernels]
  [dummy]
    type = Diffusion
    variable = sat
  []
[]

[Materials]
  [hys_order]
    type = PorousFlowHysteresisOrder
  []
  [pc_calculator]
    type = PorousFlowHystereticInfo
    alpha_d = 10.0
    alpha_w = 10.0
    n_d = 1.5
    n_w = 1.9
    S_l_min = 0.1
    S_lr = 0.2
    S_gr_max = 0.3
    Pc_max = 12.0
    low_extension_type = none
    sat_var = sat
  []
[]

[AuxVariables]
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

[VectorPostprocessors]
  [pc]
    type = LineValueSampler
    warn_discontinuous_face_values = false
    start_point = '0 0 0'
    end_point = '1 0 0'
    num_points = 10
    sort_by = x
    variable = 'sat pc'
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

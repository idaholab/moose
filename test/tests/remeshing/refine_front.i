[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 8
    ny = 4
    xmax = 2
    ymax = 1
    elem_type = TRI3
  []
  [band]
    type = SubdomainBoundingBoxGenerator
    input = gmg
    block_id = 1
    bottom_left = '0 0 0'
    top_right = '1.0 1.0 0'
  []
  [far]
    type = SubdomainBoundingBoxGenerator
    input = band
    block_id = 2
    bottom_left = '1.5 0 0'
    top_right = '2.0 1.0 0'
  []
[]

[Variables]
  [u]
  []
[]

[AuxVariables]
  [target_h]
    family = MONOMIAL
    order = CONSTANT
  []
  [elem_h]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[Functions]
  [front]
    type = ParsedFunction
    expression = '0.5 * (1 - tanh((x - 0.15 - t) / 0.05))'
  []
  [kappa]
    type = ConstantFunction
    value = 1e-3
  []
[]

[ICs]
  [u_front]
    type = FunctionIC
    variable = u
    function = front
  []
[]

[Kernels]
  [time]
    type = TimeDerivative
    variable = u
  []
  [diff]
    type = FunctionDiffusion
    variable = u
    function = kappa
  []
  [relax]
    type = Reaction
    variable = u
    rate = 100
  []
  [drive]
    type = BodyForce
    variable = u
    function = front
    value = 100
  []
[]

[AuxKernels]
  [sizing]
    type = ParsedAux
    variable = target_h
    coupled_variables = 'jump'
    constant_names = 'h_min h_max jump_ref'
    constant_expressions = '0.09 0.4 0.05'
    # target_h is clamped into [h_min, h_max]; the positive floor is what makes the splitter stop
    expression = 'max(h_min, min(h_max, h_max - (h_max - h_min) * jump / jump_ref))'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [size]
    type = ElementLengthAux
    variable = elem_h
    method = max
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Adaptivity]
  [Indicators]
    [jump]
      type = GradientJumpIndicator
      variable = u
    []
  []
[]

[Remeshing]
  [Criteria]
    [sharp_front]
      type = IndicatorThresholdCriterion
      indicator = jump
      refine_threshold = 0.05
    []
  []
  [Remeshers]
    [split]
      type = TriSplitRemesher
      sizing_variable = target_h
      max_splits_per_event = 20
    []
  []
[]

[Postprocessors]
  [remesh_count]
    type = RemeshCount
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [n_elements]
    type = NumElements
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [u_integral]
    type = ElementIntegralVariablePostprocessor
    variable = u
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [u_integral_post_remesh]
    type = ElementIntegralVariablePostprocessor
    variable = u
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
  [band_min_h]
    type = ElementExtremeValue
    variable = elem_h
    value_type = min
    block = 1
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [far_min_h]
    type = ElementExtremeValue
    variable = elem_h
    value_type = min
    block = 2
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Executioner]
  type = Transient
  num_steps = 10
  dt = 0.04
  solve_type = NEWTON
  nl_rel_tol = 1e-10
[]

[Outputs]
  csv = true
[]

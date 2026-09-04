[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 6
    xmax = 2.5
    ymax = 1.5
    elem_type = TRI3
  []
  [wake]
    type = SubdomainBoundingBoxGenerator
    input = gmg
    block_id = 1
    bottom_left = '0.75 0.25 0'
    top_right = '1.0 1.25 0'
  []
  [far]
    type = SubdomainBoundingBoxGenerator
    input = wake
    block_id = 2
    bottom_left = '2.25 0.25 0'
    top_right = '2.5 1.25 0'
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
    expression = '0.5 * (1 - tanh((x - 0.7 - 1.2 * t - 0.15 * y) / 0.04))'
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
    use_xyzt = true
    constant_names = 'h_min h_max jump_ref y_mid y_half'
    constant_expressions = '0.12 0.4 0.05 0.75 0.5'
    # Clamped into [h_min, h_max]; the positive floor stops the splitter, and the y window holds the
    # target at h_max within one element of y = 0 and y = ymax so no cavity is ever pinned there
    expression = 'max(h_min, min(h_max, h_max - (h_max - h_min) * jump / jump_ref * if(abs(y - y_mid) < y_half, 1, 0)))'
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
    [front_or_over_refined]
      type = IndicatorThresholdCriterion
      indicator = jump
      refine_threshold = 0.05
      sizing_variable = target_h
      coarsen_fraction = 0.4
    []
  []
  [Remeshers]
    [split]
      type = TriSplitRemesher
      sizing_variable = target_h
      max_splits_per_event = 20
    []
    [coarsen]
      type = PatchDelaunayRemesher
      sizing_variable = target_h
      # Sets which elements are over refined and the dead band above them, both in diameters, and
      # through a derived coarsen_fraction/sqrt(2) the shortest cavity edge; 0.5 caps the first two
      coarsen_fraction = 0.4
      quality_lower_bound = 0.05
    []
  []
[]

[Postprocessors]
  [remesh_count]
    type = RemeshCount
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
  [wake_min_h]
    type = ElementExtremeValue
    variable = elem_h
    value_type = min
    block = 1
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [wake_avg_h]
    type = ElementAverageValue
    variable = elem_h
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
  [wake_far_size_ratio]
    type = ParsedPostprocessor
    expression = 'wake_avg_h / far_min_h'
    pp_names = 'wake_avg_h far_min_h'
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Executioner]
  type = Transient
  num_steps = 18
  dt = 0.04
  solve_type = NEWTON
  nl_rel_tol = 1e-10
[]

[Outputs]
  csv = true
[]

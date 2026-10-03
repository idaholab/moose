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
  [wake_h]
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
    type = MassLumpedTimeDerivative
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
    # h_min matches the 0.04 width of the front so that the refined elements resolve it: a floor
    # wider than the front leaves the solve projecting a near-discontinuity onto linear elements,
    # which rings with Gibbs over- and undershoots of order ten percent
    constant_expressions = '0.04 0.4 0.05'
    # Deliberately unclamped below: the field falls to zero and under at the front, and the
    # remesher's 'min_element_size' is what floors the target there. 'h_min' still sets the slope.
    expression = 'min(h_max, h_max - (h_max - h_min) * jump / jump_ref)'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [size]
    type = ElementLengthAux
    variable = elem_h
    method = max
    execute_on = 'INITIAL TIMESTEP_END'
  []
  # The measured region is a position mask rather than a subdomain, because a subdomain seam pins
  # its vertices against collapse and would hold the wake minimum at the refined size forever. The
  # mask blends toward 1e6 on the elements whose quadrature points straddle the box edge, so the
  # minimum reads the elements that lie inside the box.
  [wake_size]
    type = ParsedAux
    variable = wake_h
    coupled_variables = 'elem_h'
    use_xyzt = true
    # The box is wider than the 0.4 target ceiling times sqrt(2) so that a fully coarsened element
    # still fits inside it with every quadrature point, keeping the minimum below the 1e6 mask
    expression = 'if(x > 0.55 & x < 1.2 & y > 0.2 & y < 1.3, elem_h, 1e6)'
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
  # Equilibrate the mesh to the initial condition before the transient: the background coarsens
  # toward the 0.4 ceiling and the front refines toward the floor at t = 0, so the first time
  # steps carry no equilibration transient
  initial_remesh_cycles = 5
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
    [edges]
      type = TriEdgeRemesher
      sizing_variable = target_h
      min_element_size = 0.04
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
    execute_on = 'TIMESTEP_END'
  []
  [u_integral_post_remesh]
    type = ElementIntegralVariablePostprocessor
    variable = u
    execute_on = 'TIMESTEP_BEGIN'
  []
  [wake_min_h]
    type = ElementExtremeValue
    variable = wake_h
    value_type = min
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

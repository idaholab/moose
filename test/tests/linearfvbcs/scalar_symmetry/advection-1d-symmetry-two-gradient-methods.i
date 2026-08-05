vel = 0.1

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim  = 1
    nx   = 2
    xmin = 0
    xmax = ${fparse pi}
  []
[]

[Problem]
  linear_sys_names = 'u_sys'
[]

[Variables]
  [u]
    type = MooseLinearVariableFVReal
    solver_sys = 'u_sys'
    initial_condition = 1.0
    gradient_method = gg
  []
  [v]
    type = MooseLinearVariableFVReal
    solver_sys = 'u_sys'
    initial_condition = 1.0
    gradient_method = gg_limited
  []
[]

[FVGradientMethods]
  [gg]
    type = FVGreenGaussGradient
  []
  [gg_limited]
    type = FVGreenGaussGradient
    limiter = venkatakrishnan
  []
[]

[FVInterpolationMethods]
  [average]
    type = FVGeometricAverage
  []
[]

[Functions]
  [u_exact]
    type = ParsedFunction
    expression = 'cos(x)'
  []
  [source_fn]
    type = ParsedFunction
    expression = '-${vel}*sin(x)'
  []
[]

[LinearFVKernels]
  [advection_u]
    type = LinearFVAdvection
    variable = u
    velocity = "${vel} 0 0"
    advected_interp_method_name = average
  []
  [source_u]
    type = LinearFVSource
    variable = u
    source_density = source_fn
  []
  [advection_v]
    type = LinearFVAdvection
    variable = v
    velocity = "${vel} 0 0"
    advected_interp_method_name = average
  []
  [source_v]
    type = LinearFVSource
    variable = v
    source_density = source_fn
  []
[]

[LinearFVBCs]
  [rob_l_u]
    type = LinearFVAdvectionDiffusionScalarSymmetryBC
    variable = u
    boundary = "left"
    use_two_term_expansion = true
  []
  [dirichlet_u]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = u
    boundary = "right"
    functor = u_exact
  []
  [rob_l_v]
    type = LinearFVAdvectionDiffusionScalarSymmetryBC
    variable = v
    boundary = "left"
    use_two_term_expansion = true
  []
  [dirichlet_v]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = v
    boundary = "right"
    functor = u_exact
  []
[]

[Postprocessors]
  [h]
    type = AverageElementSize
    execute_on = FINAL
  []
  [error_u]
    type = ElementL2FunctorError
    approximate = u
    exact = u_exact
    execute_on = FINAL
  []
  [error_v]
    type = ElementL2FunctorError
    approximate = v
    exact = u_exact
    execute_on = FINAL
  []
[]

[Convergence]
  [linear]
    type = IterationCountConvergence
    max_iterations = 4
    converge_at_max_iterations = true
  []
[]

[Executioner]
  type = Steady
  system_names = u_sys
  l_tol = 1e-7
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = 'lu       NONZERO               1e-10'
  linear_convergence = linear
[]

[Outputs]
  [csv]
    type = CSV
    execute_on = FINAL
  []
[]

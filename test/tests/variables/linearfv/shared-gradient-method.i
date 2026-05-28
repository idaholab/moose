[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 4
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
    gradient_method = gg
  []
[]

[FVGradientMethods]
  [gg]
    type = FVGreenGaussGradient
  []
[]

[FVInterpolationMethods]
  [muscl]
    type = FVAdvectedVenkatakrishnanDeferredCorrection
    gradient_method = gg
    deferred_correction_factor = 1.0
  []
[]

[LinearFVKernels]
  [advection_u]
    type = LinearFVAdvection
    variable = u
    velocity = "1 0 0"
    advected_interp_method_name = muscl
  []
  [source_u]
    type = LinearFVSource
    variable = u
    source_density = source_func
  []
  [advection_v]
    type = LinearFVAdvection
    variable = v
    velocity = "1 0 0"
    advected_interp_method_name = muscl
  []
  [source_v]
    type = LinearFVSource
    variable = v
    source_density = source_func
  []
[]

[LinearFVBCs]
  [inflow_u]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = u
    boundary = "left"
    functor = analytic_solution
  []
  [outflow_u]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = u
    boundary = "right"
    use_two_term_expansion = false
  []
  [inflow_v]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = v
    boundary = "left"
    functor = analytic_solution
  []
  [outflow_v]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = v
    boundary = "right"
    use_two_term_expansion = false
  []
[]

[Functions]
  [source_func]
    type = ParsedFunction
    expression = x
  []
  [analytic_solution]
    type = ParsedFunction
    expression = '0.5+0.5*x*x'
  []
[]

[Postprocessors]
  [error_u]
    type = ElementL2FunctorError
    approximate = u
    exact = analytic_solution
    execute_on = FINAL
  []
  [error_v]
    type = ElementL2FunctorError
    approximate = v
    exact = analytic_solution
    execute_on = FINAL
  []
  [h]
    type = AverageElementSize
    execute_on = FINAL
  []
[]

[Convergence]
  [linear]
    type = IterationCountConvergence
    max_iterations = 8
    converge_at_max_iterations = true
  []
[]

[Executioner]
  type = Steady
  system_names = u_sys
  l_tol = 1e-10
  multi_system_fixed_point = true
  multi_system_fixed_point_convergence = linear
  multi_system_fixed_point_relaxation_factor = 1.0
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_shift_amount -pc_factor_mat_solver_type -mat_mumps_icntl_14'
  petsc_options_value = 'lu       NONZERO               1e-12                     mumps                    50'
[]

[Outputs]
  [csv]
    type = CSV
    execute_on = FINAL
  []
[]

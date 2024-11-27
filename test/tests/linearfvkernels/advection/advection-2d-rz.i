[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny= 1
    ymax = 0.5
  []
  coord_type = RZ
  rz_coord_axis = Y
[]

[Problem]
  linear_sys_names = 'u_sys'
[]

[Variables]
  [u]
    type = MooseLinearVariableFVReal
    solver_sys = 'u_sys'
    initial_condition = 1.0
  []
[]

[LinearFVKernels]
  [advection]
    type = LinearFVAdvection
    variable = u
    velocity = "0.0 0.5 0"
    advected_interp_method = average
  []
  [source]
    type = LinearFVSource
    variable = u
    source_density = source_func
  []
[]

[LinearFVBCs]
  [inflow]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = u
    boundary = "left right bottom"
    functor = analytic_solution
  []
  [outflow]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = u
    boundary = "top"
    use_two_term_expansion = true
  []
[]

[Functions]
  [source_func]
    type = ParsedFunction
    expression = '1.0*pi*sin(x*pi)*cos(2*y*pi)'
  []
  [analytic_solution]
    type = ParsedFunction
    expression = 'sin(x*pi)*sin(2*y*pi) + 1.5'
  []
[]

[Postprocessors]
  [error]
    type = ElementL2FunctorError
    approximate = u
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
    max_iterations = 10
    converge_at_max_iterations = true
  []
[]

[Executioner]
  type = Steady
  system_names = u_sys
  l_abs_tol = 1e-10
  multi_system_fixed_point=true
  multi_system_fixed_point_convergence=linear
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = 'lu       NONZERO               1e-10'
[]

[Outputs]
  [csv]
    type = CSV
    execute_on = FINAL
  []
[]

# 2D diffusion MMS test with Neumann normal-gradient BCs on internal block boundaries.
# Exact solution:  u = v = 1.5 - x^2
# Diffusion coeff: D = 1 + 0.5*x*y

[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 2
    dx = '0.5 0.5'
    ix = '2 2'
    dy = '0.5'
    iy = '2'
    subdomain_id = '1 2'
  []
  [left_internal]
    type = SideSetsBetweenSubdomainsGenerator
    input = cmg
    new_boundary = left_internal
    primary_block = 1
    paired_block = 2
  []
  [right_internal]
    type = SideSetsBetweenSubdomainsGenerator
    input = left_internal
    new_boundary = right_internal
    primary_block = 2
    paired_block = 1
  []
[]

[Problem]
  linear_sys_names = 'u_sys'
[]

[Variables]
  [u]
    type = MooseLinearVariableFVReal
    solver_sys = 'u_sys'
    block = 1
  []
  [v]
    type = MooseLinearVariableFVReal
    solver_sys = 'u_sys'
    block = 2
  []
[]

[LinearFVKernels]
  [diffusion_left]
    type = KokkosLinearFVDiffusion
    variable = u
    diffusion_coeff = coeff_func_kokkos
    block = 1
  []
  [source_left]
    type = KokkosLinearFVSource
    variable = u
    source_density = source_func_kokkos
    scaling_factor = unit_fn
    block = 1
  []
  [diffusion_right]
    type = KokkosLinearFVDiffusion
    variable = v
    diffusion_coeff = coeff_func_kokkos
    block = 2
  []
  [source_right]
    type = KokkosLinearFVSource
    variable = v
    source_density = source_func_kokkos
    scaling_factor = unit_fn
    block = 2
  []
[]

[LinearFVBCs]
  [dirichlet_left]
    type = KokkosLinearFVFunctorDirichletBC
    variable = u
    boundary = left
    functor = analytic_solution_kokkos
  []
  [neumann_left_internal]
    type = KokkosLinearFVFunctorNeumannBC
    variable = u
    boundary = left_internal
    functor = left_internal_gradient
  []
  [dirichlet_right]
    type = KokkosLinearFVFunctorDirichletBC
    variable = v
    boundary = right
    functor = analytic_solution_kokkos
  []
  [neumann_right_internal]
    type = KokkosLinearFVFunctorNeumannBC
    variable = v
    boundary = right_internal
    functor = right_internal_gradient
  []
[]

[Functions]
  [unit_fn]
    type = KokkosParsedFunction
    expression = 1.0
  []
  [coeff_func_kokkos]
    type = KokkosParsedFunction
    expression = '1.0 + 0.5*x*y'
  []
  [source_func_kokkos]
    type = KokkosParsedFunction
    expression = '2.0 + 2.0*x*y'
  []
  [analytic_solution_kokkos]
    type = KokkosParsedFunction
    expression = '1.5 - x*x'
  []
  [analytic_solution]
    type = ParsedFunction
    expression = '1.5 - x*x'
  []
  [left_internal_gradient]
    type = KokkosParsedFunction
    expression = '-1.0'
  []
  [right_internal_gradient]
    type = KokkosParsedFunction
    expression = '1.0'
  []
[]

[Postprocessors]
  [h]
    type = AverageElementSize
    execute_on = FINAL
  []
  [left_error]
    type = ElementL2FunctorError
    approximate = u
    exact = analytic_solution
    execute_on = FINAL
    block = 1
  []
  [right_error]
    type = ElementL2FunctorError
    approximate = v
    exact = analytic_solution
    execute_on = FINAL
    block = 2
  []
  [error]
    type = ParsedPostprocessor
    expression = 'sqrt(left*left + right*right)'
    pp_names = 'left_error right_error'
    pp_symbols = 'left right'
    execute_on = FINAL
  []
[]

[Executioner]
  type = Steady
  system_names = u_sys
  l_tol = 1e-10
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  [csv]
    type = CSV
    execute_on = FINAL
  []
  [console]
    type = Console
    execute_postprocessors_on = 'FINAL'
  []
[]

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 2
  []
[]

[Problem]
  linear_sys_names = 'u_sys'
[]

[Variables]
  [u]
    type = MooseLinearVariableFVReal
    solver_sys = 'u_sys'
  []
[]

[LinearFVKernels]
  [advection]
    type = KokkosLinearFVAdvection
    variable = u
    velocity = '0.5 0 0'
  []
  [diffusion]
    type = KokkosLinearFVDiffusion
    variable = u
    diffusion_coeff = unit_fn
  []
  [source]
    type = KokkosLinearFVSource
    variable = u
    source_density = source_func_kokkos
    scaling_factor = unit_fn
  []
[]

[LinearFVBCs]
  [inflow]
    type = KokkosLinearFVFunctorDirichletBC
    variable = u
    boundary = left
    functor = analytic_solution_kokkos
  []
  [outflow]
    type = KokkosLinearFVFunctorNeumannBC
    variable = u
    boundary = right
    functor = zero_fn
  []
[]

[Functions]
  [unit_fn]
    type = KokkosParsedFunction
    expression = 1.0
  []
  [zero_fn]
    type = KokkosParsedFunction
    expression = 0.0
  []
  [source_func_kokkos]
    type = KokkosParsedFunction
    expression = 'x - 3.0'
  []
  [analytic_solution_kokkos]
    type = KokkosParsedFunction
    expression = '1.0 + (1.0 - x) * (1.0 - x)'
  []
  [analytic_solution]
    type = ParsedFunction
    expression = '1.0 + (1.0 - x) * (1.0 - x)'
  []
[]

[Postprocessors]
  [h]
    type = AverageElementSize
    execute_on = FINAL
  []
  [error]
    type = ElementL2FunctorError
    approximate = u
    exact = analytic_solution
    execute_on = FINAL
  []
[]

[Executioner]
  type = Steady
  system_names = u_sys
  l_tol = 1e-10
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_shift_amount -pc_factor_mat_solver_type -mat_mumps_icntl_14'
  petsc_options_value = 'lu       NONZERO               1e-12                     mumps                    50'
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

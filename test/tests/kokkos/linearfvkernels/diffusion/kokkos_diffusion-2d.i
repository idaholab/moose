[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 1
    ymax = 0.5
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
  [diffusion]
    type = KokkosLinearFVDiffusion
    variable = u
    diffusion_coeff = coeff_func_kokkos
  []
  [source]
    type = KokkosLinearFVSource
    variable = u
    source_density = source_func_kokkos
    scaling_factor = unit_fn
  []
[]

[LinearFVBCs]
  [dir]
    type = KokkosLinearFVFunctorDirichletBC
    variable = u
    boundary = "left right top bottom"
    functor = analytic_solution_kokkos
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
    expression = '2*(1.5-y*y) + 2*x*y*(1.5-y*y) + 2*(1.5-x*x) + 2*x*y*(1.5-x*x)'
  []
  [analytic_solution_kokkos]
    type = KokkosParsedFunction
    expression = '(1.5-x*x)*(1.5-y*y)'
  []
  [analytic_solution]
    type = ParsedFunction
    expression = '(1.5-x*x)*(1.5-y*y)'
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

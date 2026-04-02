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
    initial_condition = 1.0
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
    type = KokkosLinearFVAdvectionDiffusionFunctorDirichletBC
    variable = u
    boundary = "left right top bottom"
    functor = analytic_solution_kokkos
  []
[]

[Functions]
  [unit_fn]
    type = KokkosConstantFunction
    value = 1.0
  []
  [coeff_func_kokkos]
    type = KokkosLinearFV2DDiffusionCoefficient
  []
  [source_func_kokkos]
    type = KokkosLinearFV2DSourceFunction
  []
  [analytic_solution_kokkos]
    type = KokkosLinearFV2DExactSolution
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

[Convergence]
  [linear]
    type = IterationCountConvergence
    max_iterations = 1
    converge_at_max_iterations = true
  []
[]

[Executioner]
  type = Steady
  system_names = u_sys
  l_tol = 1e-10
  multi_system_fixed_point = true
  multi_system_fixed_point_convergence = linear
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

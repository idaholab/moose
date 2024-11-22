[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 1
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
  [diffusion]
    type = LinearFVDiffusion
    variable = u
    diffusion_coeff = coeff_func
    use_nonorthogonal_correction = true
  []
  [source]
    type = LinearFVSource
    variable = u
    source_density = source_func
  []
[]

[LinearFVBCs]
  [dir]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = u
    boundary = "left right top bottom"
    functor = analytic_solution
  []
[]

[Functions]
  [coeff_func]
    type = ParsedFunction
    expression = '1+0.5*x*y'
  []
  [source_func]
    type = ParsedFunction
    expression = '-(-1.0*x^2*y*(1.5 - x^2) + x*(1.5 - x^2)*(-1.0*x*y - 2))/x - (-1.0*x^2*y*(1.5 - y^2) - 4*x*(1.5 - y^2)*(0.5*x*y + 1))/x'
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
  l_abs_tol = 1e-10
  multi_system_fixed_point=true
  multi_system_fixed_point_convergence=linear
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  [csv]
    type = CSV
    execute_on = FINAL
  []
  [exo]
    type = Exodus
    execute_on = FINAL
  []
[]

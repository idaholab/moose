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
    initial_condition = 1.0
  []
[]

[LinearFVKernels]
  [diffusion]
    type = LinearFVDiffusion
    variable = u
    diffusion_coeff = diff_coeff_func
    use_nonorthogonal_correction = false
  []
  [advection]
    type = LinearFVAdvection
    variable = u
    velocity = "0.5 0 0"
    advected_interp_method = average
  []
  [reaction]
    type = LinearFVReaction
    variable = u
    coeff = coeff_func
  []
  [source]
    type = LinearFVSource
    variable = u
    source_density = source_func
  []
[]

[LinearFVBCs]
  inactive = "outflow"
  [dir]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = u
    boundary = "left right"
    functor = analytic_solution
  []
  [outflow]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = u
    boundary = "right"
    use_two_term_expansion = true
  []
[]

[Functions]
  [diff_coeff_func]
    type = ParsedFunction
    expression = '1+0.5*x'
  []
  [coeff_func]
    type = ParsedFunction
    expression = '1+1/(1+x)'
  []
  [source_func]
    type = ParsedFunction
    expression = '(1+1/(x+1))*(sin(pi/2*x)+1.5)+0.25*pi*pi*(0.5*x+1)*sin(pi/2*x)'
  []
  [analytic_solution]
    type = ParsedFunction
    expression = 'sin(pi/2*x)+1.5'
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
[]

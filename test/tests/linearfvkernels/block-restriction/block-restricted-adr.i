[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 2
    dx = '0.1 1 0.1'
    dy = '0.1 0.5 0.1'
    ix = '1 2 1'
    iy = '1 1 1'
    subdomain_id = '1 1 1 1 2 3 1 1 1'
  []
  [transform]
    type = TransformGenerator
    input = cmg
    transform = TRANSLATE
    vector_value = '-0.1 -0.1 0.0'
  []
  [create_sides]
    type = SideSetsBetweenSubdomainsGenerator
    input = transform
    new_boundary = sides
    primary_block = 2
    paired_block = 1
  []
  [create_outlet]
    type = SideSetsBetweenSubdomainsGenerator
    input = create_sides
    new_boundary = outlet
    primary_block = 2
    paired_block = 3
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
    block = 2
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
    boundary = "sides outlet"
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
    expression = '1.0+0.5*x*y'
  []
  [coeff_func]
    type = ParsedFunction
    expression = '1.0+1.0/(1+x*y)'
  []
  [source_func]
    type = ParsedFunction
    expression = '-1.0*x*pi*sin((1/2)*x*pi)*cos(2*y*pi) - 0.25*y*pi*sin(2*y*pi)*cos((1/2)*x*pi) + (1.0 + 1.0/(x*y + 1))*(sin((1/2)*x*pi)*sin(2*y*pi) + 1.5) + (17/4)*pi^2*(0.5*x*y + 1.0)*sin((1/2)*x*pi)*sin(2*y*pi) + 0.25*pi*sin(2*y*pi)*cos((1/2)*x*pi)'
  []
  [analytic_solution]
    type = ParsedFunction
    expression = 'sin((1/2)*x*pi)*sin(2*y*pi) + 1.5'
  []
[]

[Postprocessors]
  [h]
    type = AverageElementSize
    execute_on = FINAL
    block = 2
  []
  [error]
    type = ElementL2FunctorError
    approximate = u
    exact = analytic_solution
    execute_on = FINAL
    block = 2
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

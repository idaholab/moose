[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
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

[AuxVariables]
  [grad_u_default]
    family = MONOMIAL_VEC
    order = CONSTANT
  []
  [grad_u_limited]
    family = MONOMIAL_VEC
    order = CONSTANT
  []
[]

[FVGradientMethods]
  [limited]
    type = FVGreenGaussGradient
    limiter = venkatakrishnan
  []
[]

[LinearFVKernels]
  [diffusion]
    type = LinearFVDiffusion
    variable = u
    diffusion_coeff = 1
    use_nonorthogonal_correction = false
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
    boundary = 'left right top bottom'
    functor = analytic_solution
  []
[]

[AuxKernels]
  [grad_u_default]
    type = LinearFVGradientAux
    variable = grad_u_default
    linear_fv_variable = u
  []
  [grad_u_limited]
    type = LinearFVGradientAux
    variable = grad_u_limited
    linear_fv_variable = u
    gradient_method = limited
  []
[]

[Functions]
  [source_func]
    type = ParsedFunction
    expression = '2*(1.5-y*y)+2*(1.5-x*x)'
  []
  [analytic_solution]
    type = ParsedFunction
    expression = '(1.5-x*x)*(1.5-y*y)'
  []
  [analytic_gradient]
    type = ParsedVectorFunction
    expression_x = '-2*x*(1.5-y*y)'
    expression_y = '-2*y*(1.5-x*x)'
  []
[]

[Postprocessors]
  [default_gradient_error]
    type = ElementVectorL2Error
    variable = grad_u_default
    function = analytic_gradient
    execute_on = FINAL
  []
  [limited_gradient_error]
    type = ElementVectorL2Error
    variable = grad_u_limited
    function = analytic_gradient
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
[]

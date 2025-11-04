#################################################################
k     = 7.0 # diffusion coeff.
amp   = 3.6 # sinusoid amplitude, for u_exact

x1   = ${fparse 0.1*pi}
x2   = ${fparse 1.0*pi}
y1   = ${fparse 0.0*pi}
y2   = ${fparse 1.0*pi}

alpha = 2.000 # robin BC coeff for gradient term
beta  = 5.000 # robin BC coeff for variable term

nx = 2
ny = 2
##################################################################

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim  = 2
    nx   = ${nx}
    ny   = ${ny}
    xmin = ${x1}
    xmax = ${x2}
    ymin = ${y1}
    ymax = ${y2}
  []
[]

[Problem]
  linear_sys_names = 'u_sys'
[]

[Variables]
  [u]
    type = MooseLinearVariableFVReal
    solver_sys = 'u_sys'
    initial_condition = 0.0
  []
[]

[Functions]
  [u_exact]
    type = ParsedFunction
    expression = '(${amp}*sin(x)*sin(y))'
  []
  [source_fn]
    type = ParsedFunction
    expression = '${fparse k*amp}*2.0*sin(x)*sin(y)'
  []
  [gamma_fn]
    type = ParsedFunction
    expression = '${fparse -amp*alpha}*cos(x)*sin(y) + ${beta} * u_e'
    symbol_names = 'u_e'
    symbol_values = 'u_exact'
  []
[]

[LinearFVKernels]
  [diffusion]
    type = LinearFVDiffusion
    variable = u
    diffusion_coeff = ${k}
    use_nonorthogonal_correction = true
  []
  [source]
    type = LinearFVSource
    variable = u
    source_density = source_fn
  []
[]

[LinearFVBCs]
  [right]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = u
    boundary = "right"
    functor = u_exact
  []
  [top]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = u
    boundary = "top"
    functor = u_exact
  []
  [bottom]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = u
    boundary = "bottom"
    functor = u_exact
  []
  [robin_left]
    type = LinearFVAdvectionDiffusionFunctorRobinBC
    variable = u
    boundary = "left"
    alpha = ${alpha}
    beta  = ${beta}
    gamma = gamma_fn
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
    exact = u_exact
    execute_on = FINAL
  []
[]

[Outputs]
  [csv]
    type = CSV
    execute_on = FINAL
  []
[]

[Convergence]
  [linear]
    type = IterationCountConvergence
    max_iterations = 4
    converge_at_max_iterations = true
  []
[]

[Executioner]
  type = Steady
  system_names = u_sys
  l_tol = 1e-7
  multi_system_fixed_point = true
  multi_system_fixed_point_convergence = linear
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  linear_convergence = linear
[]

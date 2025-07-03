##################################################################
c     = 0.01 # advection veclocity (+x direction)
amp   = 1.0 # sinusoid amplitude, for u_exact
u0    = 1.2 # any positive constant > 1.0

x_l   = ${fparse 0.0*pi}
x_r   = ${fparse pi}
y_l   = ${fparse 0.0*pi}
y_r   = ${fparse 1.0*pi}

alpha = 5.000 # robin BC coeff for gradient term
beta  = 2.000 # robin BC coeff for variable term

nx = 10
ny = 10
##################################################################

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim  = 2
    nx   = ${nx}
    ny   = ${ny}
    xmin = ${x_l}
    xmax = ${x_r}
    ymin = ${y_l}
    ymax = ${y_r}
  []
[]

[Problem]
  linear_sys_names = 'u_sys'
[]

[Variables]
  [u]
    type = MooseLinearVariableFVReal
    solver_sys = 'u_sys'
    initial_condition = 0.01
  []
[]

[Functions]
  [u_exact]
    type = ParsedFunction
    expression = '${amp} * (${u0} - cos(x)) * sin(y)'
  []
  [source_fn]
    type = ParsedFunction
    expression = '${fparse c*amp} * sin(x) * sin(y)'
  []
  [gamma_fn]
    type = ParsedFunction
    expression = '(${fparse -amp*alpha}*sin(x)*sin(y)) + (${beta} * u_e)'
    symbol_names = 'u_e'
    symbol_values = 'u_exact'
  []
[]

[LinearFVKernels]
  [advection]
    type = LinearFVAdvection
    variable = u
    velocity = "${c} 0 0"
    advected_interp_method = average
  []
  [source]
    type = LinearFVSource
    variable = u
    source_density = source_fn
  []
[]

[LinearFVBCs]
  [rob_l]
    type = LinearFVAdvectionDiffusionFunctorRobinBC
    variable = u
    boundary = "left"
    alpha = ${alpha}
    beta  = ${beta}
    gamma = gamma_fn
  []
  [outflow]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = u
    boundary = "right"
    use_two_term_expansion = true
  []
  [top]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = u
    boundary = "top"
    functor = 0.0
  []
  [bottom]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = u
    boundary = "bottom"
    functor = 0.0
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

[Convergence]
  [linear]
    type = IterationCountConvergence
    max_iterations = 5
    converge_at_max_iterations = true
  []
[]

[Executioner]
  type = Steady
  system_names = u_sys
  l_tol = 1e-10
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = 'lu       NONZERO               1e-10'
[]

[Outputs]
  [csv]
    type = CSV
    execute_on = FINAL
  []
[]

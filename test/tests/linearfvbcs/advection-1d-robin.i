##################################################################
c     = 0.1 # advection veclocity (+x direction)
amp   = 7.0 # sinusoid amplitude, for u_exact

x_l   = ${fparse 0.0*pi} # domain bound (left)
x_r   = ${fparse pi}     # domain bound (right)

alpha = 5.000 # robin BC coeff for gradient term
beta  = 2.000 # robin BC coeff for variable term

u0 = 3 # some positive constant for the solution, > 1

gamma = ${fparse (-alpha*amp*sin(x_l)) + beta*amp*(u0 - cos(x_l))} # RHS of Robin BC, applied at left boundary

npts = 4
##################################################################

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim  = 1
    nx   = ${fparse npts}
    xmin = ${fparse x_l}
    xmax = ${fparse x_r}
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
    expression = '${fparse amp}*( ${fparse u0} - cos(x))'
  []
  [source_fn]
    type = ParsedFunction
    expression = '${fparse c*amp}*sin(x)'
  []
[]

[LinearFVKernels]
  [advection]
    type = LinearFVAdvection
    variable = u
    velocity = "${fparse c} 0 0"
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
    alpha = ${fparse alpha}
    beta  = ${fparse beta}
    gamma = ${fparse gamma}
  []
  [outflow]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = u
    boundary = "right"
    use_two_term_expansion = true
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
    max_iterations = 10
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

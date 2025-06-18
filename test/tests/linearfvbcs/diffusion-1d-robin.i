##################################################################
k     = 7.0 # diffusion coeff.
amp   = 3.6 # sinusoid amplitude, for u_exact

x_l   = ${fparse 0.0*pi} # domain bound (left)
x_r   = ${fparse 0.9*pi}     # domain bound (right)

alpha = 5.000 # robin BC coeff for gradient term
beta  = 2.000 # robin BC coeff for variable term
gamma = ${fparse (alpha*amp*cos(x_r) ) + (beta*amp*sin(x_r))} # RHS of Robin BC, applied at right boundary

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
    expression = '${fparse amp}*sin(x)'
  []
  [source_fn]
    type = ParsedFunction
    expression = '${fparse k*amp}*sin(x)'
  []
[]

[LinearFVKernels]
  [diffusion]
    type = LinearFVDiffusion
    variable = u
    diffusion_coeff = ${fparse k}
    use_nonorthogonal_correction = False
  []
  [source]
    type = LinearFVSource
    variable = u
    source_density = source_fn
  []
[]

[LinearFVBCs]
  [dir_r]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = u
    boundary = "left"
    functor = 0
  []
  [rob_l]
    type = LinearFVAdvectionDiffusionFunctorRobinBC
    variable = u
    boundary = "right"
    alpha = ${fparse alpha}
    beta  = ${fparse beta}
    gamma = ${fparse gamma}
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
    max_iterations = 10
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

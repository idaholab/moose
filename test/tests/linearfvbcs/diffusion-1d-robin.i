##################################################################
k     = 5.0 # diffusion coeff.
amp   = 8.0 # sinusoid amplitude, for u_exact

x_l   = 0.1*pi # domain bound (left)
x_r   = pi     # domain bound (right)

alpha = 42.0 # robin BC coeff for gradient term
beta  = 43.0 # robin BC coeff for variable term
gamma = ${alpha*amp*cos(x_l) + beta*amp*sin(x_l)} # RHS of Robin BC, applied at left boundary

##################################################################

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 30
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

[Functions]
  [u_exact]
    type = ParsedFunction
    expression = '${amp*sin(x)}'
  []
  [source_fn]
    type = ParsedFunction
    expression = '${k*amp*sin(x)}'
  []
[]

[LinearFVKernels]
  [diffusion]
    type = LinearFVDiffusion
    variable = u
    diffusion_coeff = ${k}
  []
  [source]
    type = LinearFVSource
    variable = u
    source_density = ${source_fn}
  []
[]

[LinearFVBCs]
  [dir]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = u
    boundary = "right"
    functor = 0
  []
  [neu]
    type = LinearFVAdvectionDiffusionFunctorRobinBC
    variable = u
    boundary = "left"
    alpha = ${alpha}
    beta  = ${beta} 
    gamma = ${gamma}
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
[]

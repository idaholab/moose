diff = 0.1

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim  = 1
    nx   = 2
    xmin = 0
    xmax = ${fparse pi}
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
    expression = 'cos(x)'
  []
  [source_fn]
    type = ParsedFunction
    expression = '${diff}*cos(x)'
  []
[]

[LinearFVKernels]
  [advection]
    type = LinearFVDiffusion
    variable = u
    diffusion_coeff = ${diff}
  []
  [source]
    type = LinearFVSource
    variable = u
    source_density = source_fn
  []
[]

[LinearFVBCs]
  [rob_l]
    type = LinearFVAdvectionDiffusionScalarSymmetryBC
    variable = u
    boundary = "left"
    use_two_term_expansion = true
  []
  [dirichlet]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = u
    boundary = "right"
    functor = u_exact
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
    max_iterations = 4
    converge_at_max_iterations = true
  []
[]

[Executioner]
  type = Steady
  system_names = u_sys
  l_tol = 1e-7
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  linear_convergence = linear
[]

[Outputs]
  [csv]
    type = CSV
    execute_on = FINAL
  []
[]

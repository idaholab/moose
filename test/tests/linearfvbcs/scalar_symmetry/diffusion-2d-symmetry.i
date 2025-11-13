diff = 0.1 # diffusion coeff.

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim  = 2
    nx   = 2
    ny   = 2
    xmin = 0
    xmax = ${fparse pi/3}
    ymin = 0
    ymax = ${fparse pi/3}
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
    expression = 'cos(x)*cos(y)'
  []
  [source_fn]
    type = ParsedFunction
    expression = '2*${diff}*cos(x)*cos(y)'
  []
[]

[LinearFVKernels]
  [diffusion]
    type = LinearFVDiffusion
    variable = u
    diffusion_coeff = ${diff}
    use_nonorthogonal_correction = true
  []
  [source]
    type = LinearFVSource
    variable = u
    source_density = source_fn
  []
[]

[LinearFVBCs]
  [dir]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = u
    boundary = "right top"
    functor = u_exact
  []
  [symm]
    type = LinearFVAdvectionDiffusionScalarSymmetryBC
    variable = u
    boundary = "bottom left"
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
  l_tol = 1e-7
  multi_system_fixed_point = true
  multi_system_fixed_point_convergence = linear
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  linear_convergence = linear
[]

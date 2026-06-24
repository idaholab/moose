[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 5
  []
[]

[Problem]
  nl_sys_names = 'u_sys'
  linear_sys_names = 'v_sys'
[]

[Variables]
  [u]
    solver_sys = 'u_sys'
  []
  [v]
    type = MooseLinearVariableFVReal
    solver_sys = 'v_sys'
  []
[]

[Kernels]
  [diff]
    type = KokkosDiffusion
    variable = u
  []
  [coupled]
    type = CoupledForce
    variable = u
    v = v
  []
[]

[LinearFVKernels]
  [diff]
    type = KokkosLinearFVDiffusion
    diffusion_coeff = 'unity'
    variable = v
  []
  [coupled]
    type = LinearFVSource
    source_density = u
    variable = v
  []
[]

[BCs]
  [left]
    type = KokkosDirichletBC
    boundary = 'left'
    variable = u
    value = 0
  []
  [right]
    type = KokkosDirichletBC
    boundary = 'right'
    variable = u
    value = 1
  []
[]

[LinearFVBCs]
  [left]
    type = KokkosLinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'left'
    variable = v
    functor = 'zero'
    diffusion_coeff = 'unity'
  []
  [right]
    type = KokkosLinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'right'
    variable = v
    functor = 'unity'
    diffusion_coeff = 'unity'
  []
[]

[Functions]
  [zero]
    type = KokkosParsedFunction
    expression = '0'
  []
  [unity]
    type = KokkosParsedFunction
    expression = '1'
  []
[]

[Postprocessors]
  [fe_avg]
    type = ElementAverageValue
    variable = u
    execute_on = FINAL
  []
  [fv_avg]
    type = ElementAverageFunctorPostprocessor
    functor = v
    execute_on = FINAL
    evaluation_type = CELL_AVERAGE
  []
[]

[Convergence]
  [fixed_point]
    type = IterationCountConvergence
    converge_at_max_iterations = true
    min_iterations = 10
    max_iterations = 10
  []
[]

[Executioner]
  type = Steady
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  system_names = 'u_sys v_sys'
  multi_system_fixed_point = true
  multi_system_fixed_point_convergence = fixed_point
  nl_abs_tol = 1e-12
  l_tol = 1e-12
[]

[Outputs]
  [csv]
    type = CSV
    execute_on = FINAL
  []
  [console]
    type = Console
    execute_postprocessors_on = FINAL
  []
[]

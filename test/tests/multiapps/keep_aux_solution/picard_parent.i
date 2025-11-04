[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 5
  []
[]

[Variables]
  [u]
  []
[]

[AuxVariables]
  [v]
  []
[]

[AuxKernels]
  [increment_v]
    type = ParsedAux
    execute_on = TIMESTEP_BEGIN
    expression = 'v + 1'
    coupled_variables = v
    variable = 'v'
  []
[]

[Kernels]
  [diff]
    type = CoefDiffusion
    variable = u
    coef = 0.1
  []
  [time]
    type = TimeDerivative
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Postprocessors]
  [picard_its]
    type = NumFixedPointIterations
    execute_on = 'initial timestep_end'
  []
[]

[Executioner]
  type = Transient
  num_steps = 3
  dt = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  nl_abs_tol = 1e-14

  # Set number of fixed point iterations
  fixed_point_min_its = 5
  fixed_point_max_its = 5
  accept_on_max_fixed_point_iteration = true
[]

[Outputs]
  exodus = true
  show = 'v'
[]



[MultiApps]
  [sub]
    type = TransientMultiApp
    app_type = MooseTestApp
    positions = '0 0 0'
    input_files = picard_sub.i
    clone_parent_mesh = true
    keep_aux_solution_during_restore = true
  []
[]

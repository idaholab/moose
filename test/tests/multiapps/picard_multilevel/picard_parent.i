[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  parallel_type = replicated
[]

[Variables]
  [u]
  []
[]

[AuxVariables]
  [v]
  []
[]

[Kernels]
  [diff]
    type = CoefDiffusion
    variable = u
    coef = 0.1
  []
  [coupled_force]
    type = CoupledForce
    variable = u
    v = v
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
  num_steps = 5
  dt = 1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  fixed_point_max_its = 30
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-9
  fixed_point_rel_tol = 1e-8
  fixed_point_abs_tol = 1e-9
[]

[Outputs]
  exodus = true
[]

[MultiApps]
  [sub1]
    type = TransientMultiApp
    app_type = MooseTestApp
    positions = '0 0 0'
    input_files = picard_sub.i
    execute_on = 'timestep_end'
    # The input was originally created with effectively no restore
    # see the changes made for #5554 then reverted in #28115
    no_restore = true
  []
[]

[Transfers]
  [v]
    type = MultiAppGeneralFieldNearestLocationTransfer
    from_multi_app = sub1
    source_variable = v
    variable = v
  []
[]

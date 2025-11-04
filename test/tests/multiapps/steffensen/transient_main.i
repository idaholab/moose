[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 5
  parallel_type = replicated
  uniform_refine = 1
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
  [time]
    type = TimeDerivative
    variable = u
  []
  [force_u]
    type = CoupledForce
    variable = u
    v = v
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
  [coupling_its]
    type = NumFixedPointIterations
    execute_on = 'initial timestep_end'
  []
  [unorm]
    type = ElementL2Norm
    variable = u
  []
[]

[Executioner]
  type = Transient
  num_steps = 10
  dt = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  nl_abs_tol = 1e-14

  fixed_point_algorithm = 'steffensen'
  fixed_point_max_its = 30
  transformed_variables = 'u'
[]

[Outputs]
  csv = true
  exodus = false
[]

[MultiApps]
  [sub]
    type = TransientMultiApp
    app_type = MooseTestApp
    positions = '0 0 0'
    input_files = 'transient_sub.i'
    clone_parent_mesh = true
    execute_on = 'timestep_begin'
    # The input was originally created with effectively no restore
    # see the changes made for #5554 then reverted in #28115
    no_restore = true
  []
[]

[Transfers]
  [v_from_sub]
    type = MultiAppGeneralFieldNearestLocationTransfer
    from_multi_app = sub
    source_variable = v
    variable = v
    execute_on = 'timestep_begin'
  []
  [u_to_sub]
    type = MultiAppGeneralFieldNearestLocationTransfer
    to_multi_app = sub
    source_variable = u
    variable = u
    execute_on = 'timestep_begin'
  []
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  parallel_type = replicated
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./v_begin]
  [../]
  [./v_end]
  [../]
[]

[Kernels]
  [./diff]
    type = CoefDiffusion
    variable = u
    coef = 0.1
  [../]
  [./time]
    type = TimeDerivative
    variable = u
  [../]
  [./force_u_begin]
    type = CoupledForce
    variable = u
    v = v_begin
  [../]
  [./force_u_end]
    type = CoupledForce
    variable = u
    v = v_end
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Postprocessors]
  [picard_its]
    type = NumFixedPointIterations
    execute_on = 'initial timestep_end'
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 10
  dt = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  fixed_point_max_its = 30
  nl_abs_tol = 1e-14
[]

[MultiApps]
  [./sub_begin]
    type = TransientMultiApp
    app_type = MooseTestApp
    positions = '0 0 0'
    input_files = picard_sub.i
  [../]
  [./sub_end]
    type = TransientMultiApp
    app_type = MooseTestApp
    positions = '1 1 1'
    input_files = picard_sub.i
    execute_on = 'timestep_end'
  [../]
[]

[Transfers]
  [./v_from_sub_begin]
    type = MultiAppNearestNodeTransfer
    from_multi_app = sub_begin
    source_variable = v
    variable = v_begin
  [../]
  [./u_to_sub_begin]
    type = MultiAppNearestNodeTransfer
    to_multi_app = sub_begin
    source_variable = u
    variable = u
  [../]
  [./v_from_sub_end]
    type = MultiAppNearestNodeTransfer
    from_multi_app = sub_end
    source_variable = v
    variable = v_end
  [../]
  [./u_to_sub_end]
    type = MultiAppNearestNodeTransfer
    to_multi_app = sub_end
    source_variable = u
    variable = u
  [../]
[]

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
  [dt]
    type = TimestepSize
    execute_on = 'TIMESTEP_END'
  []
  [fp_its]
    type = NumFixedPointIterations
    execute_on = 'TIMESTEP_END'
  []
[]

[Executioner]
  type = Transient
  num_steps = 5
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  fixed_point_max_its = 10
  fixed_point_rel_tol = 1e-8
  nl_abs_tol = 1e-14
  verbose = true

  [TimeStepper]
    type = FixedPointIterationAdaptiveDT
    dt_initial = 0.1
    target_iterations = 6
    target_window = 0
    increase_factor = 2.0
    decrease_factor = 0.5
  []
[]

[Outputs]
  file_base = 'increase_dt'
  [csv]
    type = CSV
    execute_on = 'TIMESTEP_END'
  []
[]

[MultiApps]
  [sub]
    type = TransientMultiApp
    app_type = MooseTestApp
    positions = '0 0 0'
    input_files = sub.i
    sub_cycling = true
  []
[]

[Transfers]
  [v_from_sub]
    type = MultiAppGeneralFieldNearestLocationTransfer
    from_multi_app = sub
    source_variable = v
    variable = v
  []
  [u_to_sub]
    type = MultiAppGeneralFieldNearestLocationTransfer
    to_multi_app = sub
    source_variable = u
    variable = u
  []
[]

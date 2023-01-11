[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./v]
  [../]
[]

[AuxVariables]
  [./v2]
  [../]
  [./v3]
  [../]
  [./w]
  [../]
[]

[AuxKernels]
  [./set_w]
    type = NormalizationAux
    variable = w
    source_variable = v
    normal_factor = 0.1
  [../]
[]

[Kernels]
  [./diff_v]
    type = Diffusion
    variable = v
  [../]
  [./coupled_force]
    type = CoupledForce
    variable = v
    v = v2
  [../]
  [./coupled_force2]
    type = CoupledForce
    variable = v
    v = v3
  [../]
  [./td_v]
    type = TimeDerivative
    variable = v
  [../]
[]

[BCs]
  [./left_v]
    type = FunctionDirichletBC
    variable = v
    boundary = left
    function = func
  [../]
  [./right_v]
    type = DirichletBC
    variable = v
    boundary = right
    value = 0
  [../]
[]

[Functions]
  [func]
    type = ParsedFunction
    expression = 'if(t < 2.5, 1, 1 / t)'
  []
[]

[Postprocessors]
  [picard_its]
    type = NumFixedPointIterations
    execute_on = 'initial timestep_end'
  [../]
  [parent_time]
    type = Receiver
    execute_on = 'timestep_end'
  []
  [parent_dt]
    type = Receiver
    execute_on = 'timestep_end'
  []
  [time]
    type = TimePostprocessor
    execute_on = 'timestep_end'
  []
  [dt]
    type = TimestepSize
    execute_on = 'timestep_end'
  []
[]

[Executioner]
  type = Transient
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  fixed_point_max_its = 2 # deliberately make it fail at 2 to test the time step rejection behavior
  nl_rel_tol = 1e-5 # loose enough to force multiple Picard iterations on this example
  l_tol = 1e-5 # loose enough to force multiple Picard iterations on this example
  fixed_point_rel_tol = 1e-8
  num_steps = 2
[]

[MultiApps]
  [./sub2]
    type = TransientMultiApp
    positions = '0 0 0'
    input_files = picard_sub2.i
    execute_on = timestep_end
  [../]
[]

[Transfers]
  [./v_to_v3]
    type = MultiAppNearestNodeTransfer
    from_multi_app = sub2
    source_variable = v
    variable = v3
  [../]
  [./w]
    type = MultiAppNearestNodeTransfer
    to_multi_app = sub2
    source_variable = w
    variable = w
  [../]
  [time_to_sub]
    type = MultiAppPostprocessorTransfer
    from_postprocessor = time
    to_postprocessor = sub_time
    to_multi_app = sub2
  []
  [dt_to_sub]
    type = MultiAppPostprocessorTransfer
    from_postprocessor = dt
    to_postprocessor = sub_dt
    to_multi_app = sub2
  []
  [matser_time_to_sub]
    type = MultiAppPostprocessorTransfer
    from_postprocessor = time
    to_postprocessor = parent_time
    to_multi_app = sub2
  []
  [parent_dt_to_sub]
    type = MultiAppPostprocessorTransfer
    from_postprocessor = dt
    to_postprocessor = parent_dt
    to_multi_app = sub2
  []
[]

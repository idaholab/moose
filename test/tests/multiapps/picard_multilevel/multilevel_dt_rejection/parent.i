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
  [./v]
  [../]
[]

[AuxKernels]
  [./set_v]
    type = FunctionAux
    variable = v
    function = 't'
  [../]
[]

[Kernels]
  [./diff]
    type = CoefDiffusion
    variable = u
    coef = 0.1
  [../]
  [./coupled_force]
    type = CoupledForce
    variable = u
    v = v
  [../]
  [./time]
    type = TimeDerivative
    variable = u
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

[Executioner]
  type = Transient
  solve_type = PJFNK
  num_steps = 2
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  fixed_point_max_its = 1
  auto_advance = false
[]

[MultiApps]
  [./sub1]
    type = TransientMultiApp
    positions = '0 0 0'
    input_files = picard_sub.i
    execute_on = 'timestep_end'
  [../]
[]

[Transfers]
  [./u_to_v2]
    type = MultiAppNearestNodeTransfer
    to_multi_app = sub1
    source_variable = u
    variable = v2
  [../]
  [time_to_sub]
    type = MultiAppPostprocessorTransfer
    from_postprocessor = time
    to_postprocessor = parent_time
    to_multi_app = sub1
  []
  [dt_to_sub]
    type = MultiAppPostprocessorTransfer
    from_postprocessor = dt
    to_postprocessor = parent_dt
    to_multi_app = sub1
  []
[]

[Postprocessors]
  [time]
    type = TimePostprocessor
    execute_on = 'timestep_end'
  []
  [dt]
    type = TimestepSize
    execute_on = 'timestep_end'
  []
[]

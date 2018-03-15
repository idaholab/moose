[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 5
  ny = 3
  nz = 2
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./from_sub]
  [../]
[]

[Functions]
  [./bc_func]
    type = ParsedFunction
    value = y+1
  [../]
[]

[Kernels]
  [./diff]
    type = CoefDiffusion
    variable = u
    coef = 0.3
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
    type = FunctionDirichletBC
    variable = u
    boundary = right
    function = bc_func
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 5
  dt = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]

[MultiApps]
  [./auto_pos]
    type = AutoPositionsMultiApp
    app_type = MooseTestApp
    execute_on = timestep_end
    input_files = sub.i
    boundary = right
  [../]
[]

[Transfers]
  [./to_sub]
    type = MultiAppVariableValueSamplePostprocessorTransfer
    direction = to_multiapp
    multi_app = auto_pos
    source_variable = u
    postprocessor = master_value
  [../]
  [./from_sub]
    type = MultiAppNearestNodeTransfer
    direction = from_multiapp
    multi_app = auto_pos
    source_variable = u
    variable = from_sub
  [../]
[]

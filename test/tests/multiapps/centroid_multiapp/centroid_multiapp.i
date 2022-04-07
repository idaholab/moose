[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 3
  ny = 3
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./x]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./y]
    family = MONOMIAL
    order = CONSTANT
  [../]
[]

[ICs]
  [./x]
    type = FunctionIC
    function = x
    variable = x
  [../]
  [./y]
    type = FunctionIC
    function = y
    variable = y
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 'left'
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = 'right'
    value = 1
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  solve_type = PJFNK
[]

[Outputs]
  exodus = true
[]

[MultiApps]
  [sub]
    type = CentroidMultiApp
    input_files = 'sub_app.i'
    output_in_position = true
  []
[]

[Transfers]
  [./incoming_x]
    type = MultiAppVariableValueSamplePostprocessorTransfer
    source_variable = x
    to_multi_app = sub
    postprocessor = incoming_x
  [../]
  [./incoming_y]
    type = MultiAppVariableValueSamplePostprocessorTransfer
    source_variable = y
    to_multi_app = sub
    postprocessor = incoming_y
  [../]
[]

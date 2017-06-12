[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = CoefDiffusion
    variable = u
    coef = 0.001
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
  num_steps = 20
  dt = 0.1
  dtmin = 0.1
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Postprocessors]
  [./average_u]
    type = ElementAverageValue
    variable = u
  [../]
  [./time]
    type = TimePostprocessor
  [../]
[]

[Controls]
  [./a_control]
    type = PythonControl
    execute_on = 'initial timestep_begin'
    controlled_vars = 'Kernels/diff/coef BCs/right/value'
    postprocessor_vars = 'time average_u'
    python_file = 'stop_control'
  [../]

[]

[Outputs]
  exodus = true
  csv = true
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 1
  ymax = 0.1
[]

[Variables]
  [./u]
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
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 1
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 10
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 10
  dt = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Postprocessors]
  [./elem_left]
    type = ElementalVariableValue
    variable = u
    elementid = 0
  []
  [./elem_right]
    type = ElementalVariableValue
    variable = u
    elementid = 9
  []
[]

[Outputs]
  csv = true
[]

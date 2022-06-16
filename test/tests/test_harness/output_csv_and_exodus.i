[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
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
  num_steps = 5
  dt = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Postprocessors]
  [./x_field]
    type = PointValue
    variable = u
    point = '0.5 0.5 0'
  [../]
  [./y_field]
    type = PointValue
    variable = u
    point = '0.25 0.25 0'
  [../]
  [./z_field]
    type = PointValue
    variable = u
    point = '0.75 0.75 0'
  [../]
[]

[Outputs]
  csv = true
  exodus = true
[]

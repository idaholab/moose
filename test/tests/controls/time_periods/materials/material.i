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
    type = MatDiffusion
    variable = u
    prop_name = 'coef'
  [../]
  [./time]
    type = TimeDerivative
    variable = u
  [../]
[]

[./Materials]
  [./mat0]
    type = GenericConstantMaterial
    prop_names = 'coef'
    prop_values = '0.1'
  [../]
  [./mat1]
    type = GenericConstantMaterial
    prop_names = 'coef'
    prop_values = '0.4'
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
  num_steps = 10
  dt = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]

[Controls]
  [./material]
    type = TimePeriod
    enable_objects = 'mat0'
    disable_objects = 'mat1'
    start_time = 0
    end_time = 0.45
  [../]
[]

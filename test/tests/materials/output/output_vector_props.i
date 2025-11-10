[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 5
  xmax = 10
  ymax = 10
  uniform_refine = 1
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = CoefDiffusion
    variable = u
    coef = 10
  []
  [time]
    type = TimeDerivative
    variable = u
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

[Materials]
  [test_material]
    type = GenericConstantStdVectorMaterial
    block = 0
    prop_names = 'test'
    prop_values = '0 1 2 3 4 5 6 7 8 9 10 11'
    outputs = all
  []
[]

[Executioner]
  type = Transient
  num_steps = 2
  dt = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
